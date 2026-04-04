// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        ImageViewer.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/ImageViewer.hpp"

#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include <Magick++.h>

#include <spdlog/spdlog.h>

#include "core/MediaService.hpp"

namespace Ui {

ImageViewer::ImageViewer(wxWindow* parent, const std::filesystem::path& path)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxHSCROLL | wxVSCROLL),
      m_zoom(1.0), m_dragging(false) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &ImageViewer::OnPaint, this);
    Bind(wxEVT_SIZE, &ImageViewer::OnSize, this);
    Bind(wxEVT_MOUSEWHEEL, &ImageViewer::OnMouseWheel, this);
    Bind(wxEVT_LEFT_DOWN, &ImageViewer::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ImageViewer::OnLeftUp, this);
    Bind(wxEVT_MOTION, &ImageViewer::OnMotion, this);
    Bind(wxEVT_RIGHT_DOWN, &ImageViewer::OnContextMenu, this);

    if (!path.empty()) {
        LoadImage(path);
    }
}

void ImageViewer::LoadImage(const std::filesystem::path& path) {
    m_path = path;

    try {
        Magick::Image img(path.string());
        img.type(Magick::TrueColorType);

        Magick::Blob blob;
        img.write(&blob, "RGB");

        auto dataSize = static_cast<int>(img.columns()) * static_cast<int>(img.rows()) * 3;
        std::vector<unsigned char> rgbData(
            static_cast<const unsigned char*>(blob.data()),
            static_cast<const unsigned char*>(blob.data()) + dataSize);

        wxImage wxImg(static_cast<int>(img.columns()),
                      static_cast<int>(img.rows()),
                      rgbData.data(),
                      true);
        m_bitmap = wxBitmap(wxImg);
        m_scaledBitmap = wxBitmap();
        m_zoom = 1.0;

        FitToWindow();

        spdlog::info("ImageViewer: loaded {} ({}x{})",
                     path.filename().string(),
                     m_bitmap.GetWidth(), m_bitmap.GetHeight());
    } catch (const Magick::Exception& e) {
        spdlog::error("ImageViewer: failed to load {}: {}",
                      path.string(), e.what());
        wxMessageBox("Failed to load image: " + std::string(e.what()),
                     "Image Error", wxOK | wxICON_ERROR, this);
    }
}

void ImageViewer::FitToWindow() {
    if (!m_bitmap.IsOk()) return;

    wxSize clientSize = GetClientSize();
    double scaleX = static_cast<double>(clientSize.GetWidth()) / m_bitmap.GetWidth();
    double scaleY = static_cast<double>(clientSize.GetHeight()) / m_bitmap.GetHeight();
    m_zoom = std::min(scaleX, scaleY);

    m_scaledBitmap = wxBitmap();
    UpdateScrollbars();
    Refresh();
}

void ImageViewer::ActualSize() {
    m_zoom = 1.0;
    m_scaledBitmap = wxBitmap();
    UpdateScrollbars();
    Refresh();
}

void ImageViewer::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);
    Render(dc);
    event.Skip();
}

void ImageViewer::OnSize(wxSizeEvent& event) {
    if (m_bitmap.IsOk()) {
        m_scaledBitmap = wxBitmap();
        UpdateScrollbars();
        Refresh();
    }
    event.Skip();
}

void ImageViewer::OnMouseWheel(wxMouseEvent& event) {
    double zoomDelta = event.GetWheelRotation() / 120.0 * 0.1;
    double oldZoom = m_zoom;
    m_zoom = std::max(0.1, std::min(10.0, m_zoom + zoomDelta));

    if (m_zoom != oldZoom) {
        m_scaledBitmap = wxBitmap();
        UpdateScrollbars();
        Refresh();
    }

    event.Skip();
}

void ImageViewer::OnLeftDown(wxMouseEvent& event) {
    if (m_zoom > 1.0) {
        m_dragging = true;
        m_dragStart = event.GetPosition();
        m_scrollStart = GetViewStart();
        CaptureMouse();
        SetCursor(wxCursor(wxCURSOR_HAND));
    }
    event.Skip();
}

void ImageViewer::OnLeftUp(wxMouseEvent& event) {
    if (m_dragging) {
        m_dragging = false;
        if (HasCapture()) ReleaseMouse();
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    event.Skip();
}

void ImageViewer::OnMotion(wxMouseEvent& event) {
    if (m_dragging) {
        wxPoint delta = event.GetPosition() - m_dragStart;
        Scroll(m_scrollStart.x - delta.x / 20,
               m_scrollStart.y - delta.y / 20);
    }
    event.Skip();
}

void ImageViewer::OnContextMenu(wxMouseEvent& event) {
    wxMenu menu;
    menu.Append(wxID_ANY, "Fit to Window");
    menu.Append(wxID_ANY, "Actual Size (100%)");

    Bind(wxEVT_MENU, [this](wxCommandEvent&) { FitToWindow(); },
         wxID_ANY);
    Bind(wxEVT_MENU, [this](wxCommandEvent&) { ActualSize(); },
         wxID_ANY);

    PopupMenu(&menu);
    event.Skip();
}

void ImageViewer::UpdateScrollbars() {
    if (!m_bitmap.IsOk()) return;

    int scaledW = static_cast<int>(m_bitmap.GetWidth() * m_zoom);
    int scaledH = static_cast<int>(m_bitmap.GetHeight() * m_zoom);

    SetVirtualSize(scaledW, scaledH);
    SetScrollRate(1, 1);
}

void ImageViewer::Render(wxDC& dc) {
    if (!m_bitmap.IsOk()) return;

    int scaledW = static_cast<int>(m_bitmap.GetWidth() * m_zoom);
    int scaledH = static_cast<int>(m_bitmap.GetHeight() * m_zoom);

    if (!m_scaledBitmap.IsOk() ||
        m_scaledBitmap.GetWidth() != scaledW ||
        m_scaledBitmap.GetHeight() != scaledH) {
        wxImage img = m_bitmap.ConvertToImage();
        img.Rescale(scaledW, scaledH, wxIMAGE_QUALITY_HIGH);
        m_scaledBitmap = wxBitmap(img);
    }

    dc.DrawBitmap(m_scaledBitmap, 0, 0, false);
}

} // namespace Ui

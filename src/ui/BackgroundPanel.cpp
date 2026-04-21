// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        BackgroundPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/BackgroundPanel.hpp"

#include <wx/dcclient.h>
#include <wx/image.h>

namespace Ui {

BackgroundPanel::BackgroundPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &BackgroundPanel::OnPaint, this);
    Bind(wxEVT_SIZE, &BackgroundPanel::OnSize, this);
}

void BackgroundPanel::SetBackgroundBitmap(const wxBitmap& bmp) {
    m_bitmap = bmp;
    m_scaledBitmap = wxBitmap();
    Refresh();
}

void BackgroundPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxSize clientSize = GetClientSize();

    if (m_bitmap.IsOk()) {
        if (!m_scaledBitmap.IsOk() || m_scaledBitmap.GetWidth() != clientSize.GetWidth() ||
            m_scaledBitmap.GetHeight() != clientSize.GetHeight()) {
            wxImage img = m_bitmap.ConvertToImage();
            img.Rescale(clientSize.GetWidth(), clientSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
            m_scaledBitmap = wxBitmap(img);
        }
        dc.DrawBitmap(m_scaledBitmap, 0, 0, false);
    }

    event.Skip();
}

void BackgroundPanel::OnSize(wxSizeEvent& event) {
    m_scaledBitmap = wxBitmap();
    Refresh();
    event.Skip();
}

} // namespace Ui

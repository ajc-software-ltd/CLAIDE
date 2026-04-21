// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        ImageViewer.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/bitmap.h>
#include <wx/scrolwin.h>

#include <filesystem>

namespace Ui {

class ImageViewer : public wxScrolledWindow
{
  public:
    ImageViewer(wxWindow* parent, const std::filesystem::path& path);

    void LoadImage(const std::filesystem::path& path);
    void FitToWindow();
    void ActualSize();

  private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnContextMenu(wxMouseEvent& event);

    void UpdateScrollbars();
    void Render(wxDC& dc);

    wxBitmap m_bitmap;
    wxBitmap m_scaledBitmap;
    std::filesystem::path m_path;
    double m_zoom;
    bool m_dragging;
    wxPoint m_dragStart;
    wxPoint m_scrollStart;
};

} // namespace Ui

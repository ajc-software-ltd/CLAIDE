// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        BackgroundPanel.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/panel.h>
#include <wx/bitmap.h>

namespace Ui {

class BackgroundPanel : public wxPanel {
public:
    BackgroundPanel(wxWindow* parent);

    void SetBackgroundBitmap(const wxBitmap& bmp);

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);

    wxBitmap m_bitmap;
    wxBitmap m_scaledBitmap;
};

} // namespace Ui

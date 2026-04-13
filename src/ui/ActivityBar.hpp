// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        ActivityBar.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/panel.h>
#include <wx/bitmap.h>

#include <functional>
#include <vector>

namespace Ui {

enum class ActivityMode {
    Notepad,
    Images,
    Video,
    Models,
    AI,
    Settings
};

class ActivityBar : public wxPanel {
public:
    ActivityBar(wxWindow* parent);

    void SetModeCallback(std::function<void(ActivityMode)> cb) { m_modeCb = std::move(cb); }
    void SetActiveMode(ActivityMode mode);
    ActivityMode GetActiveMode() const { return m_activeMode; }

private:
    void OnMouse(wxMouseEvent& event);
    void OnPaint(wxPaintEvent& event);

    struct IconEntry {
        wxBitmap bitmap;
        wxBitmap activeBitmap;
        wxBitmap inactiveBitmap;
        ActivityMode mode;
        wxRect hitRect;
    };

    std::vector<IconEntry> m_icons;
    ActivityMode m_activeMode;
    std::function<void(ActivityMode)> m_modeCb;
};

} // namespace Ui

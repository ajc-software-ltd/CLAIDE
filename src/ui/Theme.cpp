// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        Theme.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/Theme.hpp"

#include <wx/settings.h>
#include <wx/sysopt.h>

namespace Ui {

ThemeColours Theme::GetDarkTheme() {
    return ThemeColours{
        wxColour(30, 30, 30), wxColour(220, 220, 220), wxColour(45, 45, 45),  wxColour(220, 220, 220),
        wxColour(40, 40, 40), wxColour(180, 180, 180), wxColour(60, 90, 130), wxColour(240, 240, 240),
        wxColour(55, 55, 55), wxColour(220, 220, 220), wxColour(60, 60, 60),
    };
}

void Theme::ApplyDarkTheme() {
    wxSystemOptions::SetOption("msw.remap", 0);
}

} // namespace Ui

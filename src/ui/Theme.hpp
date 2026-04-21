// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        Theme.hpp
// Project:     CLAIDE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/colour.h>

namespace Ui {

struct ThemeColours
{
    wxColour background;
    wxColour text;
    wxColour menuBackground;
    wxColour menuText;
    wxColour statusBarBackground;
    wxColour statusBarText;
    wxColour selectionBackground;
    wxColour selectionText;
    wxColour buttonBackground;
    wxColour buttonText;
    wxColour border;
};

class Theme
{
  public:
    static ThemeColours GetDarkTheme();
    static void ApplyDarkTheme();
};

} // namespace Ui

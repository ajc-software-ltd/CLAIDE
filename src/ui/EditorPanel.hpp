// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        EditorPanel.hpp
// Project:     CLAIDE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/textctrl.h>

namespace Ui {

class EditorPanel : public wxTextCtrl
{
  public:
    EditorPanel(wxWindow* parent, wxWindowID id);

    void SetEditorFont(const wxFont& font);
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    void SetEditorColours(const wxColour& foreground, const wxColour& background);
};

} // namespace Ui

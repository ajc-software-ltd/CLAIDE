// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        EditorPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/EditorPanel.hpp"

#include "ui/Theme.hpp"

namespace Ui {

EditorPanel::EditorPanel(wxWindow* parent, wxWindowID id)
    : wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition,
                 wxDefaultSize,
                 wxTE_MULTILINE | wxTE_PROCESS_ENTER | wxHSCROLL) {
    auto theme = Theme::GetDarkTheme();

    wxFont font(wxFontInfo(11).Family(wxFONTFAMILY_TELETYPE));
    SetFont(font);

    SetBackgroundColour(theme.background);
    SetForegroundColour(theme.text);

    Refresh();
}

void EditorPanel::SetEditorFont(const wxFont& font) {
    SetFont(font);
    Refresh();
}

void EditorPanel::SetEditorColours(const wxColour& foreground,
                                   const wxColour& background) {
    SetForegroundColour(foreground);
    SetBackgroundColour(background);
    Refresh();
}

} // namespace Ui

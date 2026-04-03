// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        EditorWindow.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/frame.h>

#include "core/Document.hpp"

namespace Ui {

class EditorPanel;

class EditorWindow : public wxFrame {
public:
    EditorWindow(wxWindow* parent);

    void LoadFile(const std::filesystem::path& path);
    bool SaveFile(const std::filesystem::path& path);

private:
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnTextChanged(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnSelectAll(wxCommandEvent& event);

    void CreateMenuBar();
    void CreateStatusBar();
    void CreateEditor();
    void UpdateTitle();
    void UpdateStatusBar();

    Core::Document m_document;
    EditorPanel* m_editor;
    wxStatusBar* m_statusBar;
};

} // namespace Ui

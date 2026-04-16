// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        EditorDocumentController.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <functional>
#include <string>
#include <unordered_map>

#include <wx/aui/aui.h>
#include <wx/window.h>

#include "core/Document.hpp"

namespace Ui {

class EditorPanel;

class EditorDocumentController {
public:
    EditorDocumentController(
        wxWindow* parent,
        wxAuiNotebook* notebook,
        std::unordered_map<wxWindow*, Core::Document>& documents);

    void BindEditorEvents(EditorPanel* editor);
    void RefreshEditorTabTitle(wxWindow* page);
    std::expected<void, std::string> SaveDocumentForPage(
        wxWindow* page,
        bool forceSaveAs,
        const std::function<void()>& onSaved);
    bool ConfirmClosePage(wxWindow* page);

private:
    wxWindow* m_parent;
    wxAuiNotebook* m_notebook;
    std::unordered_map<wxWindow*, Core::Document>& m_documents;
};

} // namespace Ui

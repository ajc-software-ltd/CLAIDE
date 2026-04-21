// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        EditorDocumentController.hpp
// Project:     CLAIDE
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
#include "core/DocumentWorkflowService.hpp"

namespace Ui {

class EditorPanel;

class EditorDocumentController
{
  public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    EditorDocumentController(wxWindow* parent, wxAuiNotebook* notebook,
                             std::unordered_map<wxWindow*, Core::Document>& documents,
                             Core::DocumentWorkflowService& workflowService);

    void BindEditorEvents(EditorPanel* editor);
    void RefreshEditorTabTitle(wxWindow* page);
    std::expected<void, std::string> SaveDocumentForPage(wxWindow* page, bool forceSaveAs,
                                                         const std::function<void()>& onSaved);
    bool ConfirmClosePage(wxWindow* page);

  private:
    wxWindow* m_parent;
    wxAuiNotebook* m_notebook;
    std::unordered_map<wxWindow*, Core::Document>& m_documents;
    Core::DocumentWorkflowService& m_workflowService;
};

} // namespace Ui

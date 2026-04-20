// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        EditorDocumentController.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/EditorDocumentController.hpp"

#include <filesystem>

#include <spdlog/spdlog.h>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>

#include "ui/EditorPanel.hpp"

namespace Ui {

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EditorDocumentController::EditorDocumentController(wxWindow* parent, wxAuiNotebook* notebook,
                                                   std::unordered_map<wxWindow*, Core::Document>& documents,
                                                   Core::DocumentWorkflowService& workflowService)
    : m_parent(parent), m_notebook(notebook), m_documents(documents), m_workflowService(workflowService) {
}

void EditorDocumentController::BindEditorEvents(EditorPanel* editor) {
    if (editor == nullptr)
        return;

    editor->Bind(wxEVT_TEXT, [this, editor](wxCommandEvent& textEvent) {
        auto docIt = m_documents.find(editor);
        if (docIt != m_documents.end()) {
            auto newContent = editor->GetValue().ToStdString();
            if (docIt->second.GetContent() != newContent) {
                docIt->second.SetContent(newContent);
                docIt->second.SetModified(true);
                RefreshEditorTabTitle(editor);
            }
        }
        textEvent.Skip();
    });
}

void EditorDocumentController::RefreshEditorTabTitle(wxWindow* page) {
    if (m_notebook == nullptr || page == nullptr)
        return;
    auto index = m_notebook->GetPageIndex(page);
    if (index == wxNOT_FOUND)
        return;

    auto docIt = m_documents.find(page);
    if (docIt == m_documents.end())
        return;

    auto title = docIt->second.GetDisplayName();
    if (docIt->second.IsModified()) {
        title += "*";
    }
    m_notebook->SetPageText(static_cast<size_t>(index), title);
}

EditorDocumentController::SaveDocumentResult
EditorDocumentController::SaveDocumentForPage(wxWindow* page, bool forceSaveAs, const std::function<void()>& onSaved) {
    if (page == nullptr || m_notebook == nullptr) {
        return std::unexpected("No active editor page.");
    }

    auto docIt = m_documents.find(page);
    if (docIt == m_documents.end()) {
        return std::unexpected("No document state is associated with this tab.");
    }

    auto* editor = dynamic_cast<EditorPanel*>(page);
    if (editor == nullptr) {
        return std::unexpected("Current tab is not a text editor.");
    }

    auto& doc = docIt->second;
    auto editorContent = editor->GetValue().ToStdString();
    if (doc.GetContent() != editorContent) {
        doc.SetContent(editorContent);
        doc.SetModified(true);
    }

    std::filesystem::path savePath;
    const auto filePath = doc.GetFilePath();
    if (!forceSaveAs && filePath.has_value()) {
        savePath = *filePath;
    } else {
        wxFileDialog saveDialog(
            m_parent, "Save File", "", doc.GetDisplayName(),
            "Text Files (*.txt;*.md;*.cpp;*.hpp;*.c;*.h)|*.txt;*.md;*.cpp;*.hpp;*.c;*.h|All Files (*.*)|*.*",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveDialog.ShowModal() != wxID_OK) {
            return SaveOutcome::Cancelled;
        }
        savePath = std::filesystem::path(saveDialog.GetPath().ToStdString());
    }

    auto saveResult = m_workflowService.SaveTextDocument(savePath, doc.GetContent(), doc.GetEncoding());
    if (!saveResult) {
        return std::unexpected(saveResult.error());
    }

    doc.SetFilePath(savePath);
    doc.SetModified(false);
    RefreshEditorTabTitle(page);
    if (onSaved) {
        onSaved();
    }
    spdlog::info("EditorDocumentController: saved {}", savePath.string());
    return SaveOutcome::Saved;
}

bool EditorDocumentController::ConfirmClosePage(wxWindow* page) {
    if (page == nullptr)
        return true;

    auto docIt = m_documents.find(page);
    if (docIt == m_documents.end() || !docIt->second.IsModified()) {
        return true;
    }

    auto answer = wxMessageBox("This document has unsaved changes. Save before closing?", "Unsaved Changes",
                               wxYES_NO | wxCANCEL | wxICON_WARNING, m_parent);
    if (answer == wxCANCEL) {
        return false;
    }
    if (answer == wxYES) {
        auto saveResult = SaveDocumentForPage(page, false, [] {});
        if (!saveResult) {
            wxMessageBox(saveResult.error(), "Save Error", wxOK | wxICON_ERROR, m_parent);
            return false;
        }
        if (*saveResult == SaveOutcome::Cancelled) {
            return false;
        }
    }
    return true;
}

} // namespace Ui

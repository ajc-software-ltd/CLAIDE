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

#include "core/FileService.hpp"
#include "ui/EditorPanel.hpp"

namespace Ui {

EditorDocumentController::EditorDocumentController(
    wxWindow* parent,
    wxAuiNotebook* notebook,
    std::unordered_map<wxWindow*, Core::Document>& documents)
    : m_parent(parent), m_notebook(notebook), m_documents(documents) {}

void EditorDocumentController::BindEditorEvents(EditorPanel* editor) {
    if (editor == nullptr) return;

    editor->Bind(wxEVT_TEXT, [this, editor](wxCommandEvent& textEvent) {
        auto docIt = m_documents.find(editor);
        if (docIt != m_documents.end()) {
            docIt->second.SetContent(editor->GetValue().ToStdString());
            docIt->second.SetModified(true);
            RefreshEditorTabTitle(editor);
        }
        textEvent.Skip();
    });
}

void EditorDocumentController::RefreshEditorTabTitle(wxWindow* page) {
    if (m_notebook == nullptr || page == nullptr) return;
    auto index = m_notebook->GetPageIndex(page);
    if (index == wxNOT_FOUND) return;

    auto docIt = m_documents.find(page);
    if (docIt == m_documents.end()) return;

    auto title = docIt->second.GetDisplayName();
    if (docIt->second.IsModified()) {
        title += "*";
    }
    m_notebook->SetPageText(static_cast<size_t>(index), title);
}

std::expected<void, std::string> EditorDocumentController::SaveDocumentForPage(
    wxWindow* page,
    bool forceSaveAs,
    const std::function<void()>& onSaved) {
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
    doc.SetContent(editor->GetValue().ToStdString());
    doc.SetModified(true);

    std::filesystem::path savePath;
    if (!forceSaveAs && doc.GetFilePath().has_value()) {
        savePath = *doc.GetFilePath();
    } else {
        wxFileDialog saveDialog(
            m_parent,
            "Save File",
            "",
            doc.GetDisplayName(),
            "Text Files (*.txt;*.md;*.cpp;*.hpp;*.c;*.h)|*.txt;*.md;*.cpp;*.hpp;*.c;*.h|All Files (*.*)|*.*",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (saveDialog.ShowModal() != wxID_OK) {
            return std::unexpected("Save cancelled.");
        }
        savePath = std::filesystem::path(saveDialog.GetPath().ToStdString());
    }

    auto saveResult = Core::FileService::SaveFile(savePath, doc.GetContent(), doc.GetEncoding());
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
    return {};
}

bool EditorDocumentController::ConfirmClosePage(wxWindow* page) {
    if (page == nullptr) return true;

    auto docIt = m_documents.find(page);
    if (docIt == m_documents.end() || !docIt->second.IsModified()) {
        return true;
    }

    auto answer = wxMessageBox(
        "This document has unsaved changes. Save before closing?",
        "Unsaved Changes",
        wxYES_NO | wxCANCEL | wxICON_WARNING,
        m_parent);
    if (answer == wxCANCEL) {
        return false;
    }
    if (answer == wxYES) {
        auto saveResult = SaveDocumentForPage(page, false, [] {});
        if (!saveResult) {
            wxMessageBox(saveResult.error(), "Save Error", wxOK | wxICON_ERROR, m_parent);
            return false;
        }
    }
    return true;
}

} // namespace Ui

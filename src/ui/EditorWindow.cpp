// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        EditorWindow.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/EditorWindow.hpp"

#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/string.h>

#include <span>

#include <spdlog/spdlog.h>

#include "core/FileService.hpp"
#include "ui/EditorPanel.hpp"
#include "ui/Theme.hpp"

namespace Ui {

EditorWindow::EditorWindow(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "CLIADE", wxDefaultPosition,
              wxSize(900, 650)),
      m_editor(nullptr), m_statusBar(nullptr) {
    SetBackgroundColour(Theme::GetDarkTheme().background);

    CreateMenuBar();
    CreateStatusBar();
    CreateEditor();

    Bind(wxEVT_CLOSE_WINDOW, &EditorWindow::OnClose, this);
    m_editor->Bind(wxEVT_TEXT, &EditorWindow::OnTextChanged, this);

    UpdateTitle();
    UpdateStatusBar();

    spdlog::info("EditorWindow: created");
}

void EditorWindow::CreateMenuBar() {
    auto menuBar = new wxMenuBar();

    auto fileMenu = new wxMenu();
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S");
    fileMenu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_CLOSE, "&Close\tCtrl+W");
    menuBar->Append(fileMenu, "&File");

    auto editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
    editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X");
    editMenu->Append(wxID_COPY, "&Copy\tCtrl+C");
    editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_SELECTALL, "Select &All\tCtrl+A");
    menuBar->Append(editMenu, "&Edit");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &EditorWindow::OnSave, this, wxID_SAVE);
    Bind(wxEVT_MENU, &EditorWindow::OnSaveAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU,
         [this](wxCommandEvent&) { Close(); },
         wxID_CLOSE);
    Bind(wxEVT_MENU, &EditorWindow::OnUndo, this, wxID_UNDO);
    Bind(wxEVT_MENU, &EditorWindow::OnRedo, this, wxID_REDO);
    Bind(wxEVT_MENU, &EditorWindow::OnCut, this, wxID_CUT);
    Bind(wxEVT_MENU, &EditorWindow::OnCopy, this, wxID_COPY);
    Bind(wxEVT_MENU, &EditorWindow::OnPaste, this, wxID_PASTE);
    Bind(wxEVT_MENU, &EditorWindow::OnSelectAll, this, wxID_SELECTALL);
}

void EditorWindow::CreateStatusBar() {
    m_statusBar = wxFrame::CreateStatusBar(3);
    auto theme = Theme::GetDarkTheme();
    m_statusBar->SetBackgroundColour(theme.statusBarBackground);
    m_statusBar->SetForegroundColour(theme.statusBarText);

    int widths[] = {200, 200, -1};
    m_statusBar->SetStatusWidths(3, widths);

    UpdateStatusBar();
}

void EditorWindow::CreateEditor() {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    m_editor = new EditorPanel(this, wxID_ANY);
    sizer->Add(m_editor, 1, wxEXPAND);
    SetSizer(sizer);
}

void EditorWindow::UpdateTitle() {
    std::string title = m_document.GetDisplayName();
    if (m_document.IsModified()) {
        title += " *";
    }
    title += " - CLIADE";
    SetTitle(title);
}

void EditorWindow::UpdateStatusBar() {
    if (!m_statusBar) return;

    std::string encoding = std::string(Core::Encoding::EncodingName(
        m_document.GetEncoding()));
    std::string modified = m_document.IsModified() ? "Modified" : "Saved";
    std::string path = m_document.IsUntitled()
                           ? "Untitled"
                           : m_document.GetFilePath()->string();

    m_statusBar->SetStatusText(encoding, 0);
    m_statusBar->SetStatusText(modified, 1);
    m_statusBar->SetStatusText(path, 2);
}

void EditorWindow::LoadFile(const std::filesystem::path& path) {
    auto result = Core::FileService::LoadFile(path);
    if (!result) {
        wxMessageBox(result.error(), "Open Error", wxOK | wxICON_ERROR, this);
        return;
    }

    m_document.SetContent(*result);
    m_document.SetFilePath(path);

    auto data = std::vector<std::uint8_t>(result->begin(), result->end());
    auto detected = Core::Encoding::DetectEncoding(std::span(data));
    m_document.SetEncoding(detected);

    m_editor->SetValue(wxString::FromUTF8(std::string(m_document.GetContent())));
    m_document.SetModified(false);

    UpdateTitle();
    UpdateStatusBar();

    spdlog::info("EditorWindow: loaded file: {}", path.string());
}

bool EditorWindow::SaveFile(const std::filesystem::path& path) {
    auto result = Core::FileService::SaveFile(
        path, m_editor->GetValue().ToUTF8().data(),
        m_document.GetEncoding());

    if (!result) {
        wxMessageBox(result.error(), "Save Error", wxOK | wxICON_ERROR, this);
        return false;
    }

    m_document.SetFilePath(path);
    m_document.SetModified(false);

    UpdateTitle();
    UpdateStatusBar();

    spdlog::info("EditorWindow: saved file: {}", path.string());
    return true;
}

void EditorWindow::OnTextChanged(wxCommandEvent& event) {
    if (!m_document.IsModified()) {
        m_document.SetModified(true);
        UpdateTitle();
        UpdateStatusBar();
    }
    event.Skip();
}

void EditorWindow::OnSave([[maybe_unused]] wxCommandEvent& event) {
    if (m_document.IsUntitled()) {
        OnSaveAs(event);
        return;
    }

    auto path = *m_document.GetFilePath();
    SaveFile(path);
}

void EditorWindow::OnSaveAs([[maybe_unused]] wxCommandEvent& event) {
    wxFileDialog saveDialog(this, "Save File", "",
                            m_document.GetDisplayName(),
                            "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
                            wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() != wxID_OK) return;

    SaveFile(saveDialog.GetPath().ToStdString());
}

void EditorWindow::OnUndo([[maybe_unused]] wxCommandEvent& event) {
    m_editor->Undo();
}

void EditorWindow::OnRedo([[maybe_unused]] wxCommandEvent& event) {
    m_editor->Redo();
}

void EditorWindow::OnCut([[maybe_unused]] wxCommandEvent& event) {
    m_editor->Cut();
}

void EditorWindow::OnCopy([[maybe_unused]] wxCommandEvent& event) {
    m_editor->Copy();
}

void EditorWindow::OnPaste([[maybe_unused]] wxCommandEvent& event) {
    m_editor->Paste();
}

void EditorWindow::OnSelectAll([[maybe_unused]] wxCommandEvent& event) {
    m_editor->SelectAll();
}

void EditorWindow::OnClose(wxCloseEvent& event) {
    if (m_document.IsModified()) {
        auto result = wxMessageBox(
            "The document has unsaved changes. Save before closing?",
            "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

        if (result == wxCANCEL) {
            event.Veto();
            return;
        }

        if (result == wxYES) {
            if (m_document.IsUntitled()) {
                wxFileDialog saveDialog(this, "Save File", "", "",
                                        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
                                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                if (saveDialog.ShowModal() != wxID_OK) {
                    event.Veto();
                    return;
                }
                if (!SaveFile(saveDialog.GetPath().ToStdString())) {
                    event.Veto();
                    return;
                }
            } else {
                auto path = *m_document.GetFilePath();
                if (!SaveFile(path)) {
                    event.Veto();
                    return;
                }
            }
        }
    }

    spdlog::info("EditorWindow: closing");
    event.Skip();
}

} // namespace Ui

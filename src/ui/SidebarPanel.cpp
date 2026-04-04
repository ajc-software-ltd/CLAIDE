// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        SidebarPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/SidebarPanel.hpp"

#include <wx/sizer.h>

#include <spdlog/spdlog.h>

namespace Ui {

SidebarPanel::SidebarPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_currentMode(ActivityMode::Explorer),
      m_dirCtrl(nullptr), m_openFolderBtn(nullptr) {
    SetBackgroundColour(wxColour(37, 37, 38));

    auto sizer = new wxBoxSizer(wxVERTICAL);

    m_openFolderBtn = new wxButton(this, wxID_ANY, "Open Folder");
    sizer->Add(m_openFolderBtn, 0, wxEXPAND | wxALL, 4);

    m_dirCtrl = new wxGenericDirCtrl(this, wxID_ANY,
                                     wxDirDialogDefaultFolderStr,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxDIRCTRL_SHOW_FILTERS);
    sizer->Add(m_dirCtrl, 1, wxEXPAND | wxALL, 2);

    SetSizer(sizer);

    Bind(wxEVT_BUTTON, &SidebarPanel::OnOpenFolder, this,
         m_openFolderBtn->GetId());
}

void SidebarPanel::SetMode(ActivityMode mode) {
    m_currentMode = mode;

    switch (mode) {
    case ActivityMode::Explorer:
        Show();
        break;
    case ActivityMode::Images:
    case ActivityMode::Video:
    case ActivityMode::Models:
    case ActivityMode::AI:
    case ActivityMode::Settings:
        Hide();
        break;
    }
}

void SidebarPanel::OnOpenFolder(wxCommandEvent& event) {
    (void)event;

    wxDirDialog dlg(this, "Select folder to open", "",
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dlg.ShowModal() != wxID_OK) return;

    m_dirCtrl->SetPath(dlg.GetPath());
    spdlog::info("SidebarPanel: loaded folder: {}",
                 dlg.GetPath().ToStdString());
}

} // namespace Ui

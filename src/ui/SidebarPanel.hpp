// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        SidebarPanel.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/panel.h>
#include <wx/generic/dirctrlg.h>
#include <wx/button.h>

#include "ui/ActivityBar.hpp"

namespace Ui {

class SidebarPanel : public wxPanel {
public:
    SidebarPanel(wxWindow* parent);

    void SetMode(ActivityMode mode);

private:
    void OnOpenFolder(wxCommandEvent& event);

    ActivityMode m_currentMode;
    wxGenericDirCtrl* m_dirCtrl;
    wxButton* m_openFolderBtn;
};

} // namespace Ui

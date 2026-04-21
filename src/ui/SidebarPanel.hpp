// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        SidebarPanel.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/panel.h>

#include <functional>
#include <string>

namespace Ui {

class FileBrowserPanel;

class SidebarPanel : public wxPanel
{
  public:
    SidebarPanel(wxWindow* parent);

    void SetFileOpenCallback(std::function<void(const std::string&)> cb);

  private:
    FileBrowserPanel* m_browser;
};

} // namespace Ui

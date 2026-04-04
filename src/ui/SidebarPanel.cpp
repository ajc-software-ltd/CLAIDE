// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        SidebarPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/SidebarPanel.hpp"
#include "ui/FileBrowserPanel.hpp"

#include <wx/sizer.h>

namespace Ui {

SidebarPanel::SidebarPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_browser(nullptr) {
    SetBackgroundColour(wxColour(37, 37, 38));

    auto sizer = new wxBoxSizer(wxVERTICAL);
    m_browser = new FileBrowserPanel(this);
    sizer->Add(m_browser, 1, wxEXPAND);
    SetSizer(sizer);
}

void SidebarPanel::SetFileOpenCallback(std::function<void(const std::string&)> cb) {
    if (m_browser) {
        m_browser->SetFileOpenCallback(std::move(cb));
    }
}

} // namespace Ui

// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        PropertiesPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/PropertiesPanel.hpp"

#include <wx/sizer.h>

namespace Ui {

PropertiesPanel::PropertiesPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY), m_title(nullptr), m_details(nullptr) {
    SetBackgroundColour(wxColour(37, 37, 38));
    SetMinSize(wxSize(250, -1));

    auto sizer = new wxBoxSizer(wxVERTICAL);

    m_title = new wxStaticText(this, wxID_ANY, "Properties");
    m_title->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    m_title->SetForegroundColour(wxColour(200, 200, 200));
    sizer->Add(m_title, 0, wxALL, 8);

    m_details = new wxStaticText(this, wxID_ANY, "No content selected");
    m_details->SetForegroundColour(wxColour(140, 140, 140));
    m_details->Wrap(230);
    sizer->Add(m_details, 1, wxALL | wxEXPAND, 8);

    SetSizer(sizer);
}

void PropertiesPanel::SetInfo(const wxString& title, const wxString& details) {
    if (m_title)
        m_title->SetLabel(title);
    if (m_details)
        m_details->SetLabel(details);
}

} // namespace Ui

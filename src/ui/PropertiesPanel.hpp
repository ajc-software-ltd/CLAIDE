// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        PropertiesPanel.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/panel.h>
#include <wx/stattext.h>

namespace Ui {

class PropertiesPanel : public wxPanel
{
  public:
    PropertiesPanel(wxWindow* parent);

    void SetInfo(const wxString& title, const wxString& details);

  private:
    wxStaticText* m_title;
    wxStaticText* m_details;
};

} // namespace Ui

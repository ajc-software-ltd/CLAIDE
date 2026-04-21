// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        PromptBar.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/button.h>
#include <wx/panel.h>
#include <wx/textctrl.h>

#include <functional>
#include <string>

namespace Ui {

class PromptBar : public wxPanel
{
  public:
    PromptBar(wxWindow* parent);

    void SetSendCallback(std::function<void(std::string)> cb) {
        m_sendCb = std::move(cb);
    }
    void SetClearCallback(std::function<void()> cb) {
        m_clearCb = std::move(cb);
    }

  private:
    void OnSend(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);

    wxTextCtrl* m_input;
    wxButton* m_sendBtn;
    wxButton* m_clearBtn;
    std::function<void(std::string)> m_sendCb;
    std::function<void()> m_clearCb;
};

} // namespace Ui

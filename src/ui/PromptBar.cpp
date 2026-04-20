// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        PromptBar.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/PromptBar.hpp"

#include <wx/bmpbuttn.h>
#include <wx/image.h>
#include <wx/sizer.h>

#include <filesystem>

#include "platform/PlatformPaths.hpp"

namespace Ui {

PromptBar::PromptBar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_input(nullptr), m_sendBtn(nullptr), m_clearBtn(nullptr) {
    SetBackgroundColour(wxColour(35, 35, 35));

    auto projectRoot = Platform::GetProjectRoot();
    auto sendIconPath = projectRoot / "assets" / "icons" / "send_icon.png";
    auto clearIconPath = projectRoot / "assets" / "icons" / "clear_icon.png";

    wxBitmap sendBmp;
    if (std::filesystem::exists(sendIconPath)) {
        wxImage sendImg(sendIconPath.string(), wxBITMAP_TYPE_PNG);
        if (sendImg.IsOk()) {
            sendImg.Rescale(80, 32, wxIMAGE_QUALITY_HIGH);
            sendBmp = wxBitmap(sendImg);
        }
    }

    wxBitmap clearBmp;
    if (std::filesystem::exists(clearIconPath)) {
        wxImage clearImg(clearIconPath.string(), wxBITMAP_TYPE_PNG);
        if (clearImg.IsOk()) {
            clearImg.Rescale(80, 32, wxIMAGE_QUALITY_HIGH);
            clearBmp = wxBitmap(clearImg);
        }
    }

    auto sizer = new wxBoxSizer(wxHORIZONTAL);

    m_input = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_input->SetFont(wxFont(11, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_input->SetBackgroundColour(wxColour(25, 25, 25));
    m_input->SetForegroundColour(wxColour(220, 220, 220));
    sizer->Add(m_input, 1, wxEXPAND | wxALL, 4);

    if (sendBmp.IsOk()) {
        m_sendBtn = new wxBitmapButton(this, wxID_ANY, sendBmp, wxDefaultPosition, wxSize(80, 32), wxBORDER_NONE);
    } else {
        m_sendBtn = new wxButton(this, wxID_ANY, "Send", wxDefaultPosition, wxSize(80, 32), wxBORDER_NONE);
        m_sendBtn->SetBackgroundColour(wxColour(34, 120, 50));
        m_sendBtn->SetForegroundColour(wxColour(220, 220, 220));
    }
    sizer->Add(m_sendBtn, 0, wxEXPAND | wxALL, 4);

    if (clearBmp.IsOk()) {
        m_clearBtn = new wxBitmapButton(this, wxID_ANY, clearBmp, wxDefaultPosition, wxSize(80, 32), wxBORDER_NONE);
    } else {
        m_clearBtn = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxSize(80, 32), wxBORDER_NONE);
        m_clearBtn->SetBackgroundColour(wxColour(140, 30, 30));
        m_clearBtn->SetForegroundColour(wxColour(220, 220, 220));
    }
    sizer->Add(m_clearBtn, 0, wxEXPAND | wxALL, 4);

    SetSizer(sizer);

    Bind(wxEVT_BUTTON, &PromptBar::OnSend, this, m_sendBtn->GetId());
    Bind(wxEVT_BUTTON, &PromptBar::OnClear, this, m_clearBtn->GetId());
    m_input->Bind(wxEVT_TEXT_ENTER, &PromptBar::OnSend, this);
}

void PromptBar::OnSend(wxCommandEvent& event) {
    (void)event;
    auto text = m_input->GetValue().ToStdString();
    if (text.empty())
        return;

    if (m_sendCb)
        m_sendCb(text);
    m_input->Clear();
}

void PromptBar::OnClear(wxCommandEvent& event) {
    (void)event;
    if (m_clearCb)
        m_clearCb();
    m_input->Clear();
}

} // namespace Ui

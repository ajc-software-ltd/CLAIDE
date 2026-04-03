// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        MainFrame.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/MainFrame.hpp"

#include <wx/accel.h>
#include <wx/dcclient.h>
#include <wx/filedlg.h>
#include <wx/generic/dirctrlg.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/tglbtn.h>

#include <filesystem>
#include <functional>

#include <spdlog/spdlog.h>

#include "ui/EditorPanel.hpp"
#include "ui/Theme.hpp"
#include "core/FileService.hpp"
#include "platform/PlatformPaths.hpp"

namespace Ui {

// ── BackgroundPanel ────────────────────────────────────────────────

BackgroundPanel::BackgroundPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &BackgroundPanel::OnPaint, this);
    Bind(wxEVT_SIZE, &BackgroundPanel::OnSize, this);
}

void BackgroundPanel::SetBackgroundBitmap(const wxBitmap& bmp) {
    m_bitmap = bmp;
    m_scaledBitmap = wxBitmap();
    Refresh();
}

void BackgroundPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxSize clientSize = GetClientSize();

    if (m_bitmap.IsOk()) {
        if (!m_scaledBitmap.IsOk() ||
            m_scaledBitmap.GetWidth() != clientSize.GetWidth() ||
            m_scaledBitmap.GetHeight() != clientSize.GetHeight()) {
            wxImage img = m_bitmap.ConvertToImage();
            img.Rescale(clientSize.GetWidth(), clientSize.GetHeight(),
                        wxIMAGE_QUALITY_HIGH);
            m_scaledBitmap = wxBitmap(img);
        }
        dc.DrawBitmap(m_scaledBitmap, 0, 0, false);
    }

    event.Skip();
}

void BackgroundPanel::OnSize(wxSizeEvent& event) {
    m_scaledBitmap = wxBitmap();
    Refresh();
    event.Skip();
}

// ── IconBar ────────────────────────────────────────────────────────

IconBar::IconBar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(160, -1)),
      m_folderActive(false) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    auto projectRoot = Platform::GetProjectRoot();
    auto iconPath = projectRoot / "assets" / "icons" / "folder_icon_128x128.png";

    if (std::filesystem::exists(iconPath)) {
        wxImage img(iconPath.string(), wxBITMAP_TYPE_PNG);
        if (img.IsOk()) {
            m_folderBmp = wxBitmap(img);
        }
    }

    if (!m_folderBmp.IsOk()) {
        wxImage img(16, 16);
        img.SetRGB(wxRect(0, 0, 16, 16), 212, 160, 23);
        img.SetRGB(wxRect(2, 2, 12, 12), 245, 200, 66);
        m_folderBmp = wxBitmap(img);
    }

    Bind(wxEVT_LEFT_DOWN, &IconBar::OnFolder, this);
    Bind(wxEVT_PAINT, &IconBar::OnPaint, this);
}

void IconBar::OnFolder(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    int iconX = (GetSize().x - m_folderBmp.GetWidth()) / 2;
    int iconY = 10;
    wxRect iconRect(iconX, iconY, m_folderBmp.GetWidth(), m_folderBmp.GetHeight());

    if (iconRect.Contains(pos)) {
        m_folderActive = !m_folderActive;
        Refresh();
        if (m_folderCb) m_folderCb();
    }
}

void IconBar::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxSize sz = GetSize();

    dc.SetBrush(wxBrush(wxColour(45, 45, 45)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, sz.x, sz.y);

    int iconX = (sz.x - m_folderBmp.GetWidth()) / 2;
    int iconY = 10;

    if (m_folderBmp.IsOk()) {
        if (m_folderActive) {
            dc.DrawBitmap(m_folderBmp, iconX, iconY, true);
        } else {
            wxImage img = m_folderBmp.ConvertToImage();
            img = img.ConvertToGreyscale();
            dc.DrawBitmap(wxBitmap(img), iconX, iconY, true);
        }
    }

    dc.SetTextForeground(m_folderActive
                             ? wxColour(220, 220, 220)
                             : wxColour(140, 140, 140));
    dc.SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_BOLD));

    wxString label = "Files";
    wxCoord textW = 0, textH = 0;
    dc.GetTextExtent(label, &textW, &textH);

    int textX = (sz.x - textW) / 2;
    int textY = iconY + m_folderBmp.GetHeight() + 2;

    dc.DrawText(label, textX, textY);

    event.Skip();
}

// ── FileExplorerPanel ──────────────────────────────────────────────

FileExplorerPanel::FileExplorerPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_dirCtrl(nullptr) {
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto btnSizer = new wxBoxSizer(wxHORIZONTAL);

    m_openFolderBtn = new wxButton(this, wxID_ANY, "Open Folder");
    btnSizer->Add(m_openFolderBtn, 1, wxEXPAND | wxALL, 2);

    m_detachBtn = new wxButton(this, wxID_ANY, "Detach");
    btnSizer->Add(m_detachBtn, 0, wxALL, 2);

    sizer->Add(btnSizer, 0, wxEXPAND);

    m_dirCtrl = new wxGenericDirCtrl(this, wxID_ANY,
                                     wxDirDialogDefaultFolderStr,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxDIRCTRL_DIR_ONLY | wxDIRCTRL_SHOW_FILTERS);
    sizer->Add(m_dirCtrl, 1, wxEXPAND | wxALL, 2);

    SetSizer(sizer);

    Bind(wxEVT_BUTTON, &FileExplorerPanel::OnOpenFolder, this,
         m_openFolderBtn->GetId());
    Bind(wxEVT_BUTTON, &FileExplorerPanel::OnDetach, this,
         m_detachBtn->GetId());
}

void FileExplorerPanel::LoadDirectory(const std::string& path) {
    m_dirCtrl->SetPath(path);
}

void FileExplorerPanel::OnOpenFolder(wxCommandEvent& event) {
    (void)event;

    wxDirDialog dlg(this, "Select folder to open", "",
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dlg.ShowModal() != wxID_OK) return;

    LoadDirectory(dlg.GetPath().ToStdString());
    spdlog::info("FileExplorer: loaded folder: {}",
                 dlg.GetPath().ToStdString());
}

void FileExplorerPanel::OnDetach(wxCommandEvent& event) {
    (void)event;

    auto parentFrame = dynamic_cast<wxFrame*>(GetParent());
    if (!parentFrame) return;

    Hide();
    auto floatFrame = new wxFrame(parentFrame, wxID_ANY, "Explorer",
                                  wxDefaultPosition, wxSize(350, 500),
                                  wxDEFAULT_FRAME_STYLE);
    floatFrame->SetBackgroundColour(wxColour(45, 45, 45));
    floatFrame->Bind(wxEVT_CLOSE_WINDOW,
                     [floatFrame](wxCloseEvent& e) {
                         floatFrame->Destroy();
                         e.Skip();
                     });

    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto detached = new FileExplorerPanel(floatFrame);
    sizer->Add(detached, 1, wxEXPAND);
    floatFrame->SetSizer(sizer);

    floatFrame->Show();

    spdlog::info("FileExplorer: detached to floating window");
}

// ── PromptBar ──────────────────────────────────────────────────────

PromptBar::PromptBar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_input(nullptr), m_sendBtn(nullptr),
      m_clearBtn(nullptr) {
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

    m_input = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                             wxDefaultPosition, wxDefaultSize,
                             wxTE_PROCESS_ENTER);
    m_input->SetFont(wxFont(11, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
                            wxFONTWEIGHT_NORMAL));
    m_input->SetBackgroundColour(wxColour(25, 25, 25));
    m_input->SetForegroundColour(wxColour(220, 220, 220));
    sizer->Add(m_input, 1, wxEXPAND | wxALL, 4);

    if (sendBmp.IsOk()) {
        m_sendBtn = new wxBitmapButton(this, wxID_ANY, sendBmp,
                                       wxDefaultPosition, wxSize(80, 32),
                                       wxBORDER_NONE);
    } else {
        m_sendBtn = new wxButton(this, wxID_ANY, "Send",
                                 wxDefaultPosition, wxSize(80, 32),
                                 wxBORDER_NONE);
        m_sendBtn->SetBackgroundColour(wxColour(34, 120, 50));
        m_sendBtn->SetForegroundColour(wxColour(220, 220, 220));
    }
    sizer->Add(m_sendBtn, 0, wxEXPAND | wxALL, 4);

    if (clearBmp.IsOk()) {
        m_clearBtn = new wxBitmapButton(this, wxID_ANY, clearBmp,
                                        wxDefaultPosition, wxSize(80, 32),
                                        wxBORDER_NONE);
    } else {
        m_clearBtn = new wxButton(this, wxID_ANY, "Clear",
                                  wxDefaultPosition, wxSize(80, 32),
                                  wxBORDER_NONE);
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
    if (text.empty()) return;

    if (m_sendCb) m_sendCb(text);
    m_input->Clear();
}

void PromptBar::OnClear(wxCommandEvent& event) {
    (void)event;
    if (m_clearCb) m_clearCb();
    m_input->Clear();
}

// ── MainFrame ──────────────────────────────────────────────────────

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CLIADE", wxDefaultPosition,
              wxSize(1200, 800)),
      m_editorTabs(nullptr), m_bgPanel(nullptr), m_iconBar(nullptr),
      m_fileExplorer(nullptr), m_promptBar(nullptr), m_explorerVisible(false) {
    SetBackgroundColour(Theme::GetDarkTheme().background);

    CreateMenuBar();
    CreateStatusBar();
    CreateDockingSystem();
    LoadBackgroundImage();
    LoadAppIcon();

    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

    spdlog::info("MainFrame: created");
}

void MainFrame::CreateMenuBar() {
    auto menuBar = new wxMenuBar();

    auto fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "&New\tCtrl+N");
    fileMenu->Append(wxID_OPEN, "&Open...\tCtrl+O");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4");
    menuBar->Append(fileMenu, "&File");

    auto helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MainFrame::OnNew, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
}

void MainFrame::CreateStatusBar() {
    wxFrame::CreateStatusBar(3);
    auto theme = Theme::GetDarkTheme();
    GetStatusBar()->SetBackgroundColour(theme.statusBarBackground);
    GetStatusBar()->SetForegroundColour(theme.statusBarText);

    int widths[] = {200, 200, -1};
    GetStatusBar()->SetStatusWidths(3, widths);

    GetStatusBar()->SetStatusText("UTF-8", 0);
    GetStatusBar()->SetStatusText("Ready", 1);
    GetStatusBar()->SetStatusText("CLIADE v0.0.2-dev", 2);
}

void MainFrame::CreateDockingSystem() {
    m_auiManager.SetManagedWindow(this);

    m_bgPanel = new BackgroundPanel(this);
    m_auiManager.AddPane(m_bgPanel,
                         wxAuiPaneInfo()
                             .Name("Background")
                             .CenterPane()
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .MaximizeButton(false)
                             .MinimizeButton(false)
                             .PaneBorder(false));

    m_iconBar = new IconBar(this);
    m_iconBar->SetFolderCallback([this]() {
        if (m_explorerVisible) {
            HideFileExplorer();
        } else {
            ShowFileExplorer();
        }
    });
    m_auiManager.AddPane(m_iconBar,
                         wxAuiPaneInfo()
                             .Name("IconBar")
                             .Left()
                             .Layer(10)
                             .MinSize(wxSize(160, -1))
                             .BestSize(wxSize(160, -1))
                             .MaxSize(wxSize(160, -1))
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .Gripper(false)
                             .Resizable(false)
                             .Floatable(false)
                             .Dockable(true)
                             .PaneBorder(false));

    m_fileExplorer = new FileExplorerPanel(this);
    m_auiManager.AddPane(m_fileExplorer,
                         wxAuiPaneInfo()
                             .Name("FileExplorer")
                             .Left()
                             .Layer(1)
                             .MinSize(wxSize(250, 200))
                             .BestSize(wxSize(300, 400))
                             .Caption("Explorer")
                             .CloseButton(false)
                             .Gripper(true)
                             .Resizable(true)
                             .Floatable(true)
                             .Dockable(true)
                             .PinButton(true)
                             .PaneBorder(false)
                             .Hide());

    m_editorTabs = new wxAuiNotebook(this, wxID_ANY,
                                     wxDefaultPosition, wxDefaultSize,
                                     wxAUI_NB_TAB_MOVE |
                                     wxAUI_NB_TAB_SPLIT |
                                     wxAUI_NB_SCROLL_BUTTONS |
                                     wxAUI_NB_CLOSE_ON_ACTIVE_TAB |
                                     wxAUI_NB_WINDOWLIST_BUTTON);

    m_auiManager.AddPane(m_editorTabs,
                         wxAuiPaneInfo()
                             .Name("EditorTabs")
                             .Center()
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .MaximizeButton(true)
                             .MinimizeButton(false)
                             .Resizable(true)
                             .Floatable(true)
                             .Dockable(true)
                             .PaneBorder(false)
                             .Show(false));

    m_promptBar = new PromptBar(this);
    m_promptBar->SetSendCallback([this](std::string text) {
        spdlog::info("PromptBar: send: {}", text);
    });
    m_promptBar->SetClearCallback([this]() {
        spdlog::info("PromptBar: cleared");
    });

    m_auiManager.AddPane(m_promptBar,
                         wxAuiPaneInfo()
                             .Name("PromptBar")
                             .Bottom()
                             .MinSize(wxSize(-1, 40))
                             .BestSize(wxSize(-1, 48))
                             .MaxSize(wxSize(-1, 48))
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .Gripper(false)
                             .Resizable(false)
                             .Floatable(false)
                             .Dockable(true)
                             .PaneBorder(false));

    m_auiManager.Update();
}

void MainFrame::LoadBackgroundImage() {
    auto projectRoot = Platform::GetProjectRoot();
    auto imgPath = projectRoot / "assets" / "canvas.png";

    if (std::filesystem::exists(imgPath)) {
        wxImage img(imgPath.string(), wxBITMAP_TYPE_PNG);
        if (img.IsOk()) {
            m_bgPanel->SetBackgroundBitmap(wxBitmap(img));
            spdlog::debug("MainFrame: loaded background image: {}",
                          imgPath.string());
        }
    }
}

void MainFrame::LoadAppIcon() {
    auto projectRoot = Platform::GetProjectRoot();
    auto iconPath = projectRoot / "assets" / "icons" / "app_icon.png";

    if (!std::filesystem::exists(iconPath)) {
        spdlog::warn("MainFrame: app icon not found at {}", iconPath.string());
        return;
    }

    wxImage img(iconPath.string(), wxBITMAP_TYPE_PNG);
    if (!img.IsOk()) {
        spdlog::warn("MainFrame: failed to load app icon");
        return;
    }

    wxIcon icon;
    if (icon.LoadFile(iconPath.string(), wxBITMAP_TYPE_PNG)) {
        SetIcon(icon);
    } else {
        icon.CopyFromBitmap(wxBitmap(img));
        SetIcon(icon);
    }

    spdlog::info("MainFrame: loaded app icon: {} ({}x{})",
                 iconPath.string(), img.GetWidth(), img.GetHeight());
}

void MainFrame::ShowFileExplorer() {
    m_auiManager.GetPane("FileExplorer").Show();
    m_auiManager.Update();
    m_explorerVisible = true;
    spdlog::info("MainFrame: file explorer shown");
}

void MainFrame::HideFileExplorer() {
    m_auiManager.GetPane("FileExplorer").Hide();
    m_auiManager.Update();
    m_explorerVisible = false;
    spdlog::info("MainFrame: file explorer hidden");
}

void MainFrame::OnNew([[maybe_unused]] wxCommandEvent& event) {
    auto editor = new EditorPanel(m_editorTabs, wxID_ANY);
    size_t idx = m_editorTabs->GetPageCount();
    m_editorTabs->AddPage(editor, "Untitled", true);
    m_documents[idx] = Core::Document();

    if (!m_auiManager.GetPane("EditorTabs").IsShown()) {
        m_auiManager.GetPane("EditorTabs").Show();
        m_auiManager.Update();
    }

    spdlog::info("MainFrame: opened new editor");
}

void MainFrame::OnOpen([[maybe_unused]] wxCommandEvent& event) {
    wxFileDialog openDialog(this, "Open File", "", "",
                            "Text Files (*.txt)|*.txt|All Files (*.*)|*.*",
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() != wxID_OK) return;

    auto result = Core::FileService::LoadFile(
        openDialog.GetPath().ToStdString());
    if (!result) {
        wxMessageBox(result.error(), "Open Error", wxOK | wxICON_ERROR, this);
        return;
    }

    auto editor = new EditorPanel(m_editorTabs, wxID_ANY);
    editor->SetValue(wxString::FromUTF8(result->text));

    auto displayName = std::filesystem::path(
        openDialog.GetPath().ToStdString()).filename().string();
    size_t idx = m_editorTabs->GetPageCount();
    m_editorTabs->AddPage(editor, displayName, true);

    auto& doc = m_documents[idx];
    doc.SetContent(result->text);
    doc.SetFilePath(openDialog.GetPath().ToStdString());
    doc.SetEncoding(result->detectedEncoding);
    doc.SetModified(false);

    if (!m_auiManager.GetPane("EditorTabs").IsShown()) {
        m_auiManager.GetPane("EditorTabs").Show();
        m_auiManager.Update();
    }

    spdlog::info("MainFrame: opened file: {}",
                 openDialog.GetPath().ToStdString());
}

void MainFrame::OnExit([[maybe_unused]] wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnAbout([[maybe_unused]] wxCommandEvent& event) {
    wxMessageDialog dlg(this,
        wxString::FromUTF8("CLIADE\nVersion 0.0.2-dev\n\n"
                           "AJC-Software Ltd \xC2\xA9 2026\n\n"
                           "A cross-platform text editor built with C++23 and wxWidgets."),
        "About CLIADE", wxOK | wxICON_INFORMATION);
    dlg.ShowModal();
}

void MainFrame::OnClose(wxCloseEvent& event) {
    for (const auto& [idx, doc] : m_documents) {
        if (doc.IsModified()) {
            auto result = wxMessageBox(
                "There are unsaved changes in open editors. Close anyway?",
                "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_WARNING, this);
            if (result != wxYES) {
                event.Veto();
                return;
            }
            break;
        }
    }

    m_auiManager.UnInit();
    spdlog::info("MainFrame: closing");
    event.Skip();
}

} // namespace Ui

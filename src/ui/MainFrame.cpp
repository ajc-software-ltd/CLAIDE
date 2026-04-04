// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        MainFrame.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/MainFrame.hpp"

#include <wx/accel.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dcclient.h>
#include <wx/filedlg.h>
#include <wx/generic/dirctrlg.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/tglbtn.h>

#include <filesystem>
#include <functional>

#include <spdlog/spdlog.h>

#include "core/FileService.hpp"
#include "core/MediaService.hpp"
#include "platform/PlatformPaths.hpp"
#include "ui/EditorPanel.hpp"
#include "ui/ImageViewer.hpp"
#include "ui/Theme.hpp"

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
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(80, -1)),
      m_activeMode(SidebarMode::Files) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    auto projectRoot = Platform::GetProjectRoot();
    auto iconsDir = projectRoot / "assets" / "icons";

    struct IconDef {
        const char* filename;
        const char* label;
        SidebarMode mode;
    };

    IconDef defs[] = {
        {"folder_icon_128x128.png", "Files",   SidebarMode::Files},
        {"images_icon.png",         "Images",  SidebarMode::Images},
        {"video_icon.png",          "Video",   SidebarMode::Video},
        {"3dmodels_icon.png",       "3D",      SidebarMode::Models},
    };

    for (auto& def : defs) {
        auto iconPath = iconsDir / def.filename;
        IconEntry entry;
        entry.label = def.label;
        entry.mode = def.mode;

        if (std::filesystem::exists(iconPath)) {
            wxImage img(iconPath.string(), wxBITMAP_TYPE_PNG);
            if (img.IsOk()) {
                img.Rescale(48, 48, wxIMAGE_QUALITY_HIGH);
                entry.bitmap = wxBitmap(img);

                wxImage activeImg = img;
                entry.activeBitmap = wxBitmap(activeImg);

                wxImage inactiveImg = img;
                inactiveImg = inactiveImg.ConvertToGreyscale();
                entry.inactiveBitmap = wxBitmap(inactiveImg);
            }
        }

        if (!entry.bitmap.IsOk()) {
            wxImage fallback(48, 48);
            fallback.SetRGB(wxRect(0, 0, 48, 48), 100, 100, 100);
            entry.bitmap = wxBitmap(fallback);
            entry.activeBitmap = wxBitmap(fallback);
            wxImage grey = fallback;
            grey = grey.ConvertToGreyscale();
            entry.inactiveBitmap = wxBitmap(grey);
        }

        m_icons.push_back(std::move(entry));
    }

    Bind(wxEVT_LEFT_DOWN, &IconBar::OnMouse, this);
    Bind(wxEVT_PAINT, &IconBar::OnPaint, this);
}

void IconBar::OnMouse(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();

    for (auto& icon : m_icons) {
        if (icon.hitRect.Contains(pos)) {
            m_activeMode = icon.mode;
            Refresh();
            if (m_modeCb) m_modeCb(icon.mode);
            return;
        }
    }
}

void IconBar::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxSize sz = GetSize();

    dc.SetBrush(wxBrush(wxColour(45, 45, 45)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, sz.x, sz.y);

    int iconSize = 48;
    int spacing = 12;
    int y = 16;

    for (auto& icon : m_icons) {
        int x = (sz.x - iconSize) / 2;
        icon.hitRect = wxRect(x - 4, y - 4, iconSize + 8, iconSize + 32);

        if (icon.mode == m_activeMode) {
            dc.DrawBitmap(icon.activeBitmap, x, y, true);
            dc.SetTextForeground(wxColour(220, 220, 220));
        } else {
            dc.DrawBitmap(icon.inactiveBitmap, x, y, true);
            dc.SetTextForeground(wxColour(120, 120, 120));
        }

        dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                          wxFONTWEIGHT_NORMAL));
        wxCoord textW = 0, textH = 0;
        dc.GetTextExtent(icon.label, &textW, &textH);
        dc.DrawText(icon.label, (sz.x - textW) / 2,
                    y + iconSize + 4);

        y += iconSize + spacing + 20;
    }

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
                                     wxDIRCTRL_SHOW_FILTERS);
    sizer->Add(m_dirCtrl, 1, wxEXPAND | wxALL, 2);

    SetSizer(sizer);

    Bind(wxEVT_BUTTON, &FileExplorerPanel::OnOpenFolder, this,
         m_openFolderBtn->GetId());
    Bind(wxEVT_BUTTON, &FileExplorerPanel::OnDetach, this,
         m_detachBtn->GetId());

    m_dirCtrl->Bind(wxEVT_TREE_ITEM_ACTIVATED,
                    [this](wxTreeEvent& event) { OnFileActivated(event); },
                    m_dirCtrl->GetTreeCtrl()->GetId());
}

void FileExplorerPanel::OnFileActivated(wxTreeEvent& event) {
    (void)event;

    auto selectedPath = m_dirCtrl->GetFilePath();
    if (selectedPath.IsEmpty()) return;

    auto path = selectedPath.ToStdString();
    if (!std::filesystem::is_regular_file(std::filesystem::path(path))) {
        return;
    }

    spdlog::info("FileExplorer: file activated: {}", path);

    if (m_fileOpenCb) {
        try {
            m_fileOpenCb(path);
        } catch (const std::exception& e) {
            spdlog::error("FileExplorer: failed to open {}: {}", path, e.what());
            wxMessageBox("Failed to open file: " + std::string(e.what()),
                         "Open Error", wxOK | wxICON_ERROR, this);
        }
    }
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
      m_editorTabs(nullptr), m_imageViewer(nullptr), m_bgPanel(nullptr),
      m_iconBar(nullptr), m_fileExplorer(nullptr), m_promptBar(nullptr),
      m_currentMode(SidebarMode::Files) {
    SetBackgroundColour(Theme::GetDarkTheme().background);

    CreateMenuBar();
    CreateDockingSystem();
    LoadBackgroundImage();
    LoadAppIcon();

    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

    UpdateStatusBar();
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
    m_iconBar->SetModeCallback([this](SidebarMode mode) {
        SetSidebarMode(mode);
    });
    m_auiManager.AddPane(m_iconBar,
                         wxAuiPaneInfo()
                             .Name("IconBar")
                             .Left()
                             .Layer(10)
                             .MinSize(wxSize(80, -1))
                             .BestSize(wxSize(80, -1))
                             .MaxSize(wxSize(80, -1))
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .Gripper(false)
                             .Resizable(false)
                             .Floatable(false)
                             .Dockable(true)
                             .PaneBorder(false));

    m_fileExplorer = new FileExplorerPanel(this);
    m_fileExplorer->SetFileOpenCallback([this](const std::string& path) {
        auto mediaType = Core::MediaService::DetectMediaType(path);
        switch (mediaType) {
        case Core::MediaType::Image:
            OpenImage(path);
            break;
        case Core::MediaType::Text:
            OpenTextFile(path);
            break;
        case Core::MediaType::Video:
            wxMessageBox("Video player coming in Milestone 3.",
                         "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
            break;
        case Core::MediaType::Audio:
            wxMessageBox("Audio player coming in Milestone 4.",
                         "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
            break;
        case Core::MediaType::Model:
            wxMessageBox("3D model viewer coming in Milestone 5.",
                         "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
            break;
        default:
            wxMessageBox("Unsupported file type: " + path,
                         "Open Error", wxOK | wxICON_ERROR, this);
            break;
        }
    });
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

    m_imageViewer = new ImageViewer(this, "");
    m_auiManager.AddPane(m_imageViewer,
                         wxAuiPaneInfo()
                             .Name("ImageViewer")
                             .Center()
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .MaximizeButton(false)
                             .MinimizeButton(false)
                             .Resizable(true)
                             .Floatable(false)
                             .Dockable(true)
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

void MainFrame::SetSidebarMode(SidebarMode mode) {
    m_currentMode = mode;

    // Only toggle sidebar panels, never touch center pane content
    m_auiManager.GetPane("FileExplorer").Hide();

    switch (mode) {
    case SidebarMode::Files:
        m_auiManager.GetPane("FileExplorer").Show();
        spdlog::info("MainFrame: sidebar mode: Files");
        break;
    case SidebarMode::Images:
        spdlog::info("MainFrame: sidebar mode: Images");
        break;
    case SidebarMode::Video:
        wxMessageBox("Video player coming in Milestone 3.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    case SidebarMode::Models:
        wxMessageBox("3D model viewer coming in Milestone 5.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    }

    m_auiManager.Update();
    UpdateStatusBar();
}

void MainFrame::OpenImage(const std::filesystem::path& path) {
    try {
        m_auiManager.GetPane("Background").Hide();
        m_auiManager.GetPane("EditorTabs").Hide();
        m_auiManager.GetPane("ImageViewer").Show();
        m_auiManager.Update();

        m_imageViewer->LoadImage(path);
        m_currentMode = SidebarMode::Images;
        m_iconBar->Refresh();

        auto meta = Core::MediaService::GetImageMetadata(path);
        auto statusBar = GetStatusBar();
        if (meta && statusBar) {
            statusBar->SetStatusText(
                std::to_string(meta->width) + "x" + std::to_string(meta->height), 0);
            statusBar->SetStatusText(meta->format, 1);
            statusBar->SetStatusText(path.filename().string(), 2);
        }

        spdlog::info("MainFrame: opened image: {}", path.string());
    } catch (const std::exception& e) {
        spdlog::error("MainFrame: failed to open image {}: {}", path.string(), e.what());
        wxMessageBox("Failed to open image: " + std::string(e.what()),
                     "Image Error", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::OpenTextFile(const std::filesystem::path& path) {
    auto result = Core::FileService::LoadFile(path);
    if (!result) {
        wxMessageBox(result.error(), "Open Error", wxOK | wxICON_ERROR, this);
        return;
    }

    m_auiManager.GetPane("Background").Hide();
    m_auiManager.GetPane("ImageViewer").Hide();
    m_auiManager.GetPane("EditorTabs").Show();
    m_auiManager.Update();

    auto editor = new EditorPanel(m_editorTabs, wxID_ANY);
    editor->SetValue(wxString::FromUTF8(result->text));

    auto displayName = std::filesystem::path(path).filename().string();
    m_editorTabs->AddPage(editor, displayName, true);

    auto& doc = m_documents[m_editorTabs->GetPageCount() - 1];
    doc.SetContent(result->text);
    doc.SetFilePath(path);
    doc.SetEncoding(result->detectedEncoding);
    doc.SetModified(false);

    spdlog::info("MainFrame: opened text file: {}", path.string());
}

void MainFrame::UpdateStatusBar() {
    auto statusBar = GetStatusBar();
    if (!statusBar) return;

    switch (m_currentMode) {
    case SidebarMode::Files:
        statusBar->SetStatusText("Files", 0);
        statusBar->SetStatusText("Ready", 1);
        break;
    case SidebarMode::Images:
        statusBar->SetStatusText("Images", 0);
        statusBar->SetStatusText("Ready", 1);
        break;
    case SidebarMode::Video:
        statusBar->SetStatusText("Video", 0);
        statusBar->SetStatusText("Coming soon", 1);
        break;
    case SidebarMode::Models:
        statusBar->SetStatusText("3D Models", 0);
        statusBar->SetStatusText("Coming soon", 1);
        break;
    }
    statusBar->SetStatusText("CLIADE v0.0.2-dev", 2);
}

void MainFrame::OnNew([[maybe_unused]] wxCommandEvent& event) {
    m_auiManager.GetPane("Background").Hide();
    m_auiManager.GetPane("ImageViewer").Hide();
    m_auiManager.GetPane("EditorTabs").Show();
    m_auiManager.Update();

    auto editor = new EditorPanel(m_editorTabs, wxID_ANY);
    m_editorTabs->AddPage(editor, "Untitled", true);
    m_documents[m_editorTabs->GetPageCount() - 1] = Core::Document();

    spdlog::info("MainFrame: opened new editor");
}

void MainFrame::OnOpen([[maybe_unused]] wxCommandEvent& event) {
    wxFileDialog openDialog(this, "Open File", "", "",
                            "All Supported Files|"
                            "*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.tif;*.webp;*.psd;*.psb;*.ico;*.xpm;"
                            "*.mp4;*.webm;*.mkv;*.avi;*.mov;*.mp3;*.wav;*.ogg;*.flac;"
                            "*.fbx;*.obj;*.gltf;*.glb;*.stl;*.dae;"
                            "*.txt;*.md;*.cpp;*.hpp;*.c;*.h;*.py;*.js;*.json;*.xml"
                            "|Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.webp;*.psd;*.psb)|*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.tif;*.webp;*.psd;*.psb"
                            "|Video Files (*.mp4;*.webm;*.mkv;*.avi;*.mov)|*.mp4;*.webm;*.mkv;*.avi;*.mov"
                            "|Audio Files (*.mp3;*.wav;*.ogg;*.flac)|*.mp3;*.wav;*.ogg;*.flac"
                            "|3D Models (*.fbx;*.obj;*.gltf;*.glb)|*.fbx;*.obj;*.gltf;*.glb"
                            "|Text Files (*.txt;*.md;*.cpp;*.hpp)|*.txt;*.md;*.cpp;*.hpp"
                            "|All Files (*.*)|*.*",
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openDialog.ShowModal() != wxID_OK) return;

    auto path = openDialog.GetPath().ToStdString();
    auto mediaType = Core::MediaService::DetectMediaType(path);

    switch (mediaType) {
    case Core::MediaType::Image:
        OpenImage(path);
        break;
    case Core::MediaType::Text:
        OpenTextFile(path);
        break;
    case Core::MediaType::Video:
        wxMessageBox("Video player coming in Milestone 3.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    case Core::MediaType::Audio:
        wxMessageBox("Audio player coming in Milestone 4.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    case Core::MediaType::Model:
        wxMessageBox("3D model viewer coming in Milestone 5.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    default:
        wxMessageBox("Unsupported file type: " + path,
                     "Open Error", wxOK | wxICON_ERROR, this);
        break;
    }
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

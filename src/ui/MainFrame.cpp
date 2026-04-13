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
#include "ui/ActivityBar.hpp"
#include "ui/BackgroundPanel.hpp"
#include "ui/EditorPanel.hpp"
#include "ui/ImageViewer.hpp"
#include "ui/PropertiesPanel.hpp"
#include "ui/PromptBar.hpp"
#include "ui/Theme.hpp"

namespace Ui {

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CLIADE", wxDefaultPosition,
              wxSize(1400, 900)),
      m_editorTabs(nullptr), m_imageViewer(nullptr), m_bgPanel(nullptr),
      m_activityBar(nullptr), m_propertiesPanel(nullptr),
      m_promptBar(nullptr), m_currentMode(ActivityMode::Notepad) {
    SetBackgroundColour(Theme::GetDarkTheme().background);
    SetMinSize(wxSize(800, 600));

    CreateMenuBar();
    CreateStatusBar(3);
    CreateDockingSystem();
    LoadBackgroundImage();
    LoadAppIcon();

    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

    UpdateStatusBar();
    spdlog::info("MainFrame: created (M0 commercial layout)");
}

void MainFrame::CreateMenuBar() {
    auto menuBar = new wxMenuBar();

    auto fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "&New\tCtrl+N");
    fileMenu->Append(wxID_OPEN, "&Open...\tCtrl+O");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4");
    menuBar->Append(fileMenu, "&File");

    auto editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "&Undo\tCtrl+Z");
    editMenu->Append(wxID_REDO, "&Redo\tCtrl+Y");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, "Cu&t\tCtrl+X");
    editMenu->Append(wxID_COPY, "&Copy\tCtrl+C");
    editMenu->Append(wxID_PASTE, "&Paste\tCtrl+V");
    menuBar->Append(editMenu, "&Edit");

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

    // Background panel (center, shows canvas.png when empty)
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

    // Activity bar (far left, 48px, fixed)
    m_activityBar = new ActivityBar(this);
    m_activityBar->SetModeCallback([this](ActivityMode mode) {
        OnActivityModeChanged(mode);
    });
    m_auiManager.AddPane(m_activityBar,
                         wxAuiPaneInfo()
                             .Name("ActivityBar")
                             .Left()
                             .Layer(0)
                             .MinSize(wxSize(48, -1))
                             .BestSize(wxSize(48, -1))
                             .MaxSize(wxSize(48, -1))
                             .CaptionVisible(false)
                             .CloseButton(false)
                             .Gripper(false)
                             .Resizable(false)
                             .Floatable(false)
                             .Dockable(true)
                             .PaneBorder(false));

    // Image viewer (center, hidden until image opened)
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

    // Editor tabs (center, hidden until text opened)
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

    // Properties panel (right)
    m_propertiesPanel = new PropertiesPanel(this);
    m_auiManager.AddPane(m_propertiesPanel,
                         wxAuiPaneInfo()
                             .Name("Properties")
                             .Right()
                             .MinSize(wxSize(250, -1))
                             .BestSize(wxSize(280, -1))
                             .Caption("Properties")
                             .CloseButton(false)
                             .Gripper(true)
                             .Resizable(true)
                             .Floatable(true)
                             .Dockable(true)
                             .PinButton(true)
                             .PaneBorder(false));

    // Prompt bar (bottom)
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

void MainFrame::OnActivityModeChanged(ActivityMode mode) {
    m_currentMode = mode;

    // Hide all center content, show background by default
    m_auiManager.GetPane("ImageViewer").Hide();
    m_auiManager.GetPane("EditorTabs").Hide();
    m_auiManager.GetPane("Background").Show();

    m_auiManager.Update();
    UpdateStatusBar();

    spdlog::info("MainFrame: activity mode changed to {}",
                 static_cast<int>(mode));
}

void MainFrame::OpenImage(const std::filesystem::path& path) {
    try {
        m_auiManager.GetPane("Background").Hide();
        m_auiManager.GetPane("EditorTabs").Hide();
        m_auiManager.GetPane("ImageViewer").Show();
        m_auiManager.Update();

        m_imageViewer->LoadImage(path);
        m_currentMode = ActivityMode::Images;
        m_activityBar->Refresh();

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
    editor->SetValue(wxString::FromUTF8(std::string(result->text)));

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
    case ActivityMode::Notepad:
        statusBar->SetStatusText("Notepad", 0);
        statusBar->SetStatusText("Ready", 1);
        break;
    case ActivityMode::Images:
        statusBar->SetStatusText("Images", 0);
        statusBar->SetStatusText("Ready", 1);
        break;
    case ActivityMode::Video:
        statusBar->SetStatusText("Video", 0);
        statusBar->SetStatusText("Coming soon", 1);
        break;
    case ActivityMode::Models:
        statusBar->SetStatusText("3D Models", 0);
        statusBar->SetStatusText("Coming soon", 1);
        break;
    case ActivityMode::AI:
        statusBar->SetStatusText("AI", 0);
        statusBar->SetStatusText("Coming soon", 1);
        break;
    case ActivityMode::Settings:
        statusBar->SetStatusText("Settings", 0);
        statusBar->SetStatusText("Ready", 1);
        break;
    }
    statusBar->SetStatusText("CLIADE v0.0.3-dev", 2);
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
        wxMessageBox("Video player coming in Milestone 5.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    case Core::MediaType::Audio:
        wxMessageBox("Audio player coming in Milestone 5.",
                     "Not Yet Implemented", wxOK | wxICON_INFORMATION, this);
        break;
    case Core::MediaType::Model:
        wxMessageBox("3D model viewer coming in Milestone 6.",
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
        wxString::FromUTF8("CLIADE\nVersion 0.0.3-dev\n\n"
                           "AJC-Software Ltd \xC2\xA9 2026\n\n"
                           "AI-powered AIO IDE for code, media creation, "
                           "and content generation."),
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

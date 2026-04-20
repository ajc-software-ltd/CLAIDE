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
#include <wx/dnd.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textdlg.h>
#include <wx/tglbtn.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>

#include <spdlog/spdlog.h>

#include "core/FileSystemService.hpp"
#include "core/FileService.hpp"
#include "core/MediaService.hpp"
#include "platform/PlatformPaths.hpp"
#include "ui/ActivityBar.hpp"
#include "ui/BackgroundPanel.hpp"
#include "ui/CanvasPanel.hpp"
#include "ui/EditorDocumentController.hpp"
#include "ui/EditorPanel.hpp"
#include "ui/ImageViewer.hpp"
#include "ui/PromptBar.hpp"
#include "ui/PropertiesPanel.hpp"
#include "ui/Theme.hpp"

namespace Ui {

#ifndef CLIADE_VERSION_STRING
#define CLIADE_VERSION_STRING "0.0.56-dev"
#endif

namespace {

constexpr int kOpenRecentBaseId = wxID_HIGHEST + 1000;
constexpr int kOpenRecentMaxItems = 10;
constexpr int kQuickOpenMenuId = wxID_HIGHEST + 2000;
constexpr int kRuntimeDiagnosticsMenuId = wxID_HIGHEST + 2001;
constexpr int kRetryRuntimeMenuId = wxID_HIGHEST + 2002;
constexpr int kDeleteFileMenuId = wxID_HIGHEST + 2003;

bool IsSaveCancelled(std::string_view message) {
    return message.find("cancelled") != std::string_view::npos || message.find("canceled") != std::string_view::npos;
}

class MainFrameFileDropTarget : public wxFileDropTarget
{
  public:
    explicit MainFrameFileDropTarget(MainFrame* frame) : m_frame(frame) {
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool OnDropFiles([[maybe_unused]] wxCoord x, [[maybe_unused]] wxCoord y, const wxArrayString& filenames) override {
        if (m_frame == nullptr)
            return false;

        for (const auto& file : filenames) {
            m_frame->OpenDroppedFile(std::filesystem::path(file.ToStdString()));
        }
        return true;
    }

  private:
    MainFrame* m_frame;
};

const char* AvailabilityReasonCodeName(VulkanAIAvailabilityReasonCode code) {
    switch (code) {
    case VULKANAI_REASON_NONE:
        return "None";
    case VULKANAI_REASON_RUNTIME_NOT_LINKED:
        return "Runtime not linked";
    case VULKANAI_REASON_API_MISMATCH:
        return "API mismatch";
    case VULKANAI_REASON_UNKNOWN:
    default:
        return "Unknown";
    }
}

} // namespace

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CLIADE", wxDefaultPosition, wxSize(1400, 900)), m_editorTabs(nullptr),
      m_imageViewer(nullptr), m_canvasPanel(nullptr), m_bgPanel(nullptr), m_activityBar(nullptr),
      m_propertiesPanel(nullptr), m_promptBar(nullptr), m_editorController(nullptr), m_openRecentMenu(nullptr),
      m_currentMode(ActivityMode::Notepad), m_vulkanRenderHost(nullptr),
      m_vulkanStatus("Vulkan runtime: not initialized"), m_vulkanFrameLoopDisabled(false) {
    SetBackgroundColour(Theme::GetDarkTheme().background);
    SetMinSize(wxSize(800, 600));

    CreateMenuBar();
    CreateStatusBar(3);
    CreateDockingSystem();
    m_editorController =
        std::make_unique<EditorDocumentController>(this, m_editorTabs, m_documents, m_documentWorkflowService);
    LoadBackgroundImage();
    LoadAppIcon();

    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
    Bind(wxEVT_SIZE, &MainFrame::OnWindowResized, this);
    Bind(wxEVT_IDLE, &MainFrame::OnIdle, this);
    SetDropTarget(new MainFrameFileDropTarget(this));

    InitializeVulkanRuntime();
    UpdateStatusBar();
    spdlog::info("MainFrame: created (Milestone 2 stream layout)");
}

void MainFrame::OpenDroppedFile(const std::filesystem::path& path) {
    OpenPathUnified(path);
}

void MainFrame::CreateMenuBar() {
    auto menuBar = new wxMenuBar();

    auto fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "&New\tCtrl+N");
    fileMenu->Append(wxID_OPEN, "&Open...\tCtrl+O");
    fileMenu->Append(kQuickOpenMenuId, "Quick &Open...\tCtrl+P");
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S");
    fileMenu->Append(wxID_SAVEAS, "Save &As...\tCtrl+Shift+S");
    fileMenu->Append(kDeleteFileMenuId, "&Delete File");
    m_openRecentMenu = new wxMenu();
    fileMenu->AppendSubMenu(m_openRecentMenu, "Open &Recent");
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
    helpMenu->Append(kRuntimeDiagnosticsMenuId, "Runtime &Diagnostics");
    helpMenu->Append(kRetryRuntimeMenuId, "&Retry Vulkan Runtime");
    helpMenu->AppendSeparator();
    helpMenu->Append(wxID_ABOUT, "&About");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MainFrame::OnNew, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::OnQuickOpen, this, kQuickOpenMenuId);
    Bind(wxEVT_MENU, &MainFrame::OnSave, this, wxID_SAVE);
    Bind(wxEVT_MENU, &MainFrame::OnSaveAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &MainFrame::OnDeleteFile, this, kDeleteFileMenuId);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateSaveUi, this, wxID_SAVE);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateSaveAsUi, this, wxID_SAVEAS);
    Bind(wxEVT_UPDATE_UI, &MainFrame::OnUpdateDeleteFileUi, this, kDeleteFileMenuId);
    Bind(wxEVT_MENU, &MainFrame::OnOpenRecent, this, kOpenRecentBaseId, kOpenRecentBaseId + 50);
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::OnRuntimeDiagnostics, this, kRuntimeDiagnosticsMenuId);
    Bind(wxEVT_MENU, &MainFrame::OnRetryRuntime, this, kRetryRuntimeMenuId);
    Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);

    RebuildOpenRecentMenu();
}

void MainFrame::CreateDockingSystem() {
    m_auiManager.SetManagedWindow(this);

    // Background panel (center, shows canvas.png when empty)
    m_bgPanel = new BackgroundPanel(this);
    m_auiManager.AddPane(m_bgPanel, wxAuiPaneInfo()
                                        .Name("Background")
                                        .CenterPane()
                                        .CaptionVisible(false)
                                        .CloseButton(false)
                                        .MaximizeButton(false)
                                        .MinimizeButton(false)
                                        .PaneBorder(false));

    // Activity bar (far left, 48px, fixed)
    m_activityBar = new ActivityBar(this);
    m_activityBar->SetModeCallback([this](ActivityMode mode) { OnActivityModeChanged(mode); });
    m_auiManager.AddPane(m_activityBar, wxAuiPaneInfo()
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
    m_auiManager.AddPane(m_imageViewer, wxAuiPaneInfo()
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

    m_canvasPanel = new CanvasPanel(this);
    m_auiManager.AddPane(m_canvasPanel, wxAuiPaneInfo()
                                            .Name("CanvasPanel")
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
    m_editorTabs = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                     wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_SPLIT | wxAUI_NB_SCROLL_BUTTONS |
                                         wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_WINDOWLIST_BUTTON);
    m_auiManager.AddPane(m_editorTabs, wxAuiPaneInfo()
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
    m_editorTabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &MainFrame::OnEditorTabClosed, this);

    // Properties panel (right)
    m_propertiesPanel = new PropertiesPanel(this);
    m_auiManager.AddPane(m_propertiesPanel, wxAuiPaneInfo()
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
    m_promptBar->SetSendCallback([this](std::string text) { spdlog::info("PromptBar: send: {}", text); });
    m_promptBar->SetClearCallback([this]() { spdlog::info("PromptBar: cleared"); });
    m_auiManager.AddPane(m_promptBar, wxAuiPaneInfo()
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

void MainFrame::InitializeVulkanRuntime() {
    m_vulkanFrameLoopDisabled = false;

    auto loadResult = m_vulkanRuntime.Load();
    if (!loadResult) {
        m_vulkanStatus = "Vulkan runtime module missing";
        spdlog::warn("MainFrame: {}. Install Vulkan runtime: {}", loadResult.error(),
                     "https://vulkan.lunarg.com/sdk/home");
        return;
    }

    if (!m_vulkanRuntime.IsApiCompatible()) {
        m_vulkanStatus = "Vulkan runtime API mismatch";
        spdlog::warn("MainFrame: Vulkan runtime API mismatch");
        return;
    }

    if (!m_vulkanRuntime.IsVulkanAvailable()) {
        m_vulkanStatus = "Vulkan unavailable";
        spdlog::warn("MainFrame: Vulkan unavailable: {}. Install info: {}", m_vulkanRuntime.GetAvailabilityReason(),
                     m_vulkanRuntime.GetInstallHelpUrl());
        return;
    }

    auto initResult = m_vulkanRuntime.Initialize();
    if (initResult != VULKANAI_OK) {
        m_vulkanStatus = "Vulkan init failed";
        spdlog::warn("MainFrame: Vulkan runtime initialization failed: {}", m_vulkanRuntime.GetLastError());
        return;
    }

    m_vulkanStatus = "Vulkan runtime ready";
    m_vulkanRenderHost = std::make_unique<Core::VulkanRenderHost>(&m_vulkanRuntime);
    auto attachResult = m_vulkanRenderHost->Attach(
        Core::RenderHostConfig{.nativeWindowHandle = reinterpret_cast<std::uintptr_t>(GetHandle()),
                               .width = static_cast<uint32_t>(std::max(1, GetClientSize().GetWidth())),
                               .height = static_cast<uint32_t>(std::max(1, GetClientSize().GetHeight()))});
    if (!attachResult) {
        m_vulkanStatus = "Vulkan host attach failed";
        spdlog::warn("MainFrame: {}", attachResult.error());
        m_vulkanRenderHost.reset();
        return;
    }

    spdlog::info("MainFrame: Vulkan runtime initialized");
}

void MainFrame::ResetVulkanRuntimeState() {
    if (m_vulkanRenderHost != nullptr) {
        m_vulkanRenderHost->Detach();
        m_vulkanRenderHost.reset();
    }
    m_vulkanRuntime.Shutdown();
    m_vulkanRuntime.Unload();
    m_vulkanFrameLoopDisabled = false;
}

void MainFrame::OnWindowResized(wxSizeEvent& event) {
    if (m_vulkanRenderHost != nullptr) {
        auto size = event.GetSize();
        auto resizeResult = m_vulkanRenderHost->Resize(static_cast<uint32_t>(std::max(1, size.GetWidth())),
                                                       static_cast<uint32_t>(std::max(1, size.GetHeight())));
        if (!resizeResult) {
            spdlog::warn("MainFrame::OnWindowResized: {}", resizeResult.error());
        }
    }
    event.Skip();
}

void MainFrame::OnIdle(wxIdleEvent& event) {
    if (m_vulkanRenderHost != nullptr && !m_vulkanFrameLoopDisabled) {
        auto beginResult = m_vulkanRenderHost->BeginFrame();
        if (!beginResult) {
            HandleVulkanFrameFailure(beginResult.error());
            event.Skip();
            return;
        }

        auto endResult = m_vulkanRenderHost->EndFrame();
        if (!endResult) {
            HandleVulkanFrameFailure(endResult.error());
            event.Skip();
            return;
        }

        auto presentResult = m_vulkanRenderHost->Present();
        if (!presentResult) {
            HandleVulkanFrameFailure(presentResult.error());
            event.Skip();
            return;
        }
    }
    event.Skip();
}

void MainFrame::HandleVulkanFrameFailure(std::string_view reason) {
    m_vulkanFrameLoopDisabled = true;
    std::string reasonText(reason);
    if (reasonText.size() > 96) {
        reasonText = reasonText.substr(0, 93) + "...";
    }
    m_vulkanStatus = "Vulkan disabled: " + reasonText;
    UpdateStatusBar();
    spdlog::warn("MainFrame: disabling Vulkan frame loop after runtime error: {}", reason);

    if (m_vulkanRenderHost != nullptr) {
        m_vulkanRenderHost->Detach();
        m_vulkanRenderHost.reset();
    }
    m_vulkanRuntime.Shutdown();
    m_vulkanRuntime.Unload();
}

void MainFrame::LoadBackgroundImage() {
    auto projectRoot = Platform::GetProjectRoot();
    auto imgPath = projectRoot / "assets" / "canvas.png";

    if (Core::FileSystemService::PathExists(imgPath)) {
        wxImage img(imgPath.string(), wxBITMAP_TYPE_PNG);
        if (img.IsOk()) {
            m_bgPanel->SetBackgroundBitmap(wxBitmap(img));
            spdlog::debug("MainFrame: loaded background image: {}", imgPath.string());
        }
    }
}

void MainFrame::LoadAppIcon() {
    auto projectRoot = Platform::GetProjectRoot();
    auto iconPath = projectRoot / "assets" / "icons" / "app_icon.png";

    if (!Core::FileSystemService::PathExists(iconPath)) {
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

    spdlog::info("MainFrame: loaded app icon: {} ({}x{})", iconPath.string(), img.GetWidth(), img.GetHeight());
}

void MainFrame::OnActivityModeChanged(ActivityMode mode) {
    m_currentMode = mode;

    // Hide all center content, then show pane for active mode
    m_auiManager.GetPane("ImageViewer").Hide();
    m_auiManager.GetPane("CanvasPanel").Hide();
    m_auiManager.GetPane("EditorTabs").Hide();
    m_auiManager.GetPane("Background").Hide();

    // NOLINTNEXTLINE(bugprone-branch-clone)
    switch (mode) {
    case ActivityMode::Notepad:
        // NOLINTNEXTLINE(bugprone-branch-clone)
        if (m_editorTabs && m_editorTabs->GetPageCount() > 0) {
            m_auiManager.GetPane("EditorTabs").Show();
        } else {
            m_auiManager.GetPane("Background").Show();
        }
        break;
    case ActivityMode::Images:
    case ActivityMode::Video:
    case ActivityMode::Audio:
    case ActivityMode::Models:
    case ActivityMode::AI:
    case ActivityMode::Settings:
        m_auiManager.GetPane("CanvasPanel").Show();
        break;
    }

    m_auiManager.Update();
    UpdateStatusBar();

    spdlog::info("MainFrame: activity mode changed to {}", static_cast<int>(mode));
}

void MainFrame::OpenImage(const std::filesystem::path& path) {
    try {
        m_auiManager.GetPane("Background").Hide();
        m_auiManager.GetPane("CanvasPanel").Hide();
        m_auiManager.GetPane("EditorTabs").Hide();
        m_auiManager.GetPane("ImageViewer").Show();
        m_auiManager.Update();

        m_imageViewer->LoadImage(path);
        m_currentMode = ActivityMode::Images;
        m_activityBar->SetActiveMode(ActivityMode::Images);

        auto meta = Core::MediaService::GetImageMetadata(path);
        auto statusBar = GetStatusBar();
        if (meta && statusBar) {
            statusBar->SetStatusText(std::to_string(meta->width) + "x" + std::to_string(meta->height), 0);
            statusBar->SetStatusText(meta->format, 1);
            statusBar->SetStatusText(path.filename().string(), 2);
        }

        spdlog::info("MainFrame: opened image: {}", path.string());
    } catch (const std::exception& e) {
        spdlog::error("MainFrame: failed to open image {}: {}", path.string(), e.what());
        wxMessageBox("Failed to open image: " + std::string(e.what()), "Image Error", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::OpenTextFile(const std::filesystem::path& path) {
    auto result = m_documentWorkflowService.LoadTextDocument(path);
    if (!result) {
        wxMessageBox(result.error(), "Open Error", wxOK | wxICON_ERROR, this);
        return;
    }

    m_auiManager.GetPane("Background").Hide();
    m_auiManager.GetPane("CanvasPanel").Hide();
    m_auiManager.GetPane("ImageViewer").Hide();
    m_auiManager.GetPane("EditorTabs").Show();
    m_auiManager.Update();

    auto editor = new EditorPanel(m_editorTabs, wxID_ANY);
    editor->SetValue(wxString::FromUTF8(result->content));
    if (m_editorController != nullptr) {
        m_editorController->BindEditorEvents(editor);
    }

    auto displayName = std::filesystem::path(path).filename().string();
    m_editorTabs->AddPage(editor, displayName, true);

    auto* activePage = m_editorTabs->GetCurrentPage();
    if (activePage == nullptr) {
        spdlog::error("MainFrame: failed to obtain active editor page for {}", path.string());
        wxMessageBox("Opened file but failed to attach document state.", "Internal Error", wxOK | wxICON_ERROR, this);
        return;
    }

    auto& doc = m_documents[activePage];
    doc.SetContent(result->content);
    doc.SetFilePath(path);
    doc.SetEncoding(result->encoding);
    doc.SetModified(false);

    m_currentMode = ActivityMode::Notepad;
    m_activityBar->SetActiveMode(ActivityMode::Notepad);
    UpdateStatusBar();

    spdlog::info("MainFrame: opened text file: {}", path.string());
}

void MainFrame::OpenPathUnified(const std::filesystem::path& path, bool addToRecent) {
    if (!m_documentWorkflowService.PathExists(path)) {
        wxMessageBox("File does not exist: " + path.string(), "Open Error", wxOK | wxICON_ERROR, this);
        return;
    }

    auto mediaType = Core::MediaService::DetectMediaType(path);
    switch (mediaType) {
    case Core::MediaType::Image:
        OpenImage(path);
        break;
    case Core::MediaType::Text:
        OpenTextFile(path);
        break;
    case Core::MediaType::Video:
        m_auiManager.GetPane("Background").Hide();
        m_auiManager.GetPane("ImageViewer").Hide();
        m_auiManager.GetPane("EditorTabs").Hide();
        m_auiManager.GetPane("CanvasPanel").Show();
        m_canvasPanel->SetStatusText("Video workflow placeholder (Canvas connected)");
        m_currentMode = ActivityMode::Video;
        m_activityBar->SetActiveMode(ActivityMode::Video);
        m_auiManager.Update();
        UpdateStatusBar();
        break;
    case Core::MediaType::Audio:
        m_auiManager.GetPane("Background").Hide();
        m_auiManager.GetPane("ImageViewer").Hide();
        m_auiManager.GetPane("EditorTabs").Hide();
        m_auiManager.GetPane("CanvasPanel").Show();
        m_canvasPanel->SetStatusText("Audio workflow placeholder (Canvas connected)");
        m_currentMode = ActivityMode::Audio;
        m_activityBar->SetActiveMode(ActivityMode::Audio);
        m_auiManager.Update();
        UpdateStatusBar();
        break;
    case Core::MediaType::Model:
        m_auiManager.GetPane("Background").Hide();
        m_auiManager.GetPane("ImageViewer").Hide();
        m_auiManager.GetPane("EditorTabs").Hide();
        m_auiManager.GetPane("CanvasPanel").Show();
        m_canvasPanel->SetStatusText("3D model workflow placeholder (Canvas connected)");
        m_currentMode = ActivityMode::Models;
        m_activityBar->SetActiveMode(ActivityMode::Models);
        m_auiManager.Update();
        UpdateStatusBar();
        break;
    default:
        wxMessageBox("Unsupported file type: " + path.string(), "Open Error", wxOK | wxICON_ERROR, this);
        return;
    }

    if (addToRecent) {
        AddRecentFile(path);
    }
}

void MainFrame::AddRecentFile(const std::filesystem::path& path) {
    auto canonical = path.lexically_normal();
    m_recentFiles.erase(std::remove(m_recentFiles.begin(), m_recentFiles.end(), canonical), m_recentFiles.end());
    m_recentFiles.push_front(canonical);
    while (m_recentFiles.size() > kOpenRecentMaxItems) {
        m_recentFiles.pop_back();
    }
    RebuildOpenRecentMenu();
}

void MainFrame::RebuildOpenRecentMenu() {
    if (m_openRecentMenu == nullptr)
        return;

    while (m_openRecentMenu->GetMenuItemCount() > 0) {
        auto* item = m_openRecentMenu->FindItemByPosition(0);
        if (item == nullptr)
            break;
        m_openRecentMenu->Destroy(item);
    }
    if (m_recentFiles.empty()) {
        auto* item = m_openRecentMenu->Append(wxID_ANY, "(Empty)");
        item->Enable(false);
        return;
    }

    for (size_t i = 0; i < m_recentFiles.size(); ++i) {
        auto label = wxString::Format("&%zu %s", i + 1, m_recentFiles[i].string());
        m_openRecentMenu->Append(kOpenRecentBaseId + static_cast<int>(i), label);
    }
}

void MainFrame::UpdateStatusBar() {
    auto statusBar = GetStatusBar();
    if (!statusBar)
        return;

    switch (m_currentMode) {
    case ActivityMode::Notepad:
        statusBar->SetStatusText("Notepad", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    case ActivityMode::Images:
        statusBar->SetStatusText("Images", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    case ActivityMode::Video:
        statusBar->SetStatusText("Video", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    case ActivityMode::Audio:
        statusBar->SetStatusText("Audio", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    case ActivityMode::Models:
        statusBar->SetStatusText("3D Models", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    case ActivityMode::AI:
        statusBar->SetStatusText("AI", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    case ActivityMode::Settings:
        statusBar->SetStatusText("Settings", 0);
        statusBar->SetStatusText(m_vulkanStatus, 1);
        break;
    }
    statusBar->SetStatusText(wxString::Format("CLIADE v%s", CLIADE_VERSION_STRING), 2);
}

void MainFrame::OnNew([[maybe_unused]] wxCommandEvent& event) {
    m_auiManager.GetPane("Background").Hide();
    m_auiManager.GetPane("CanvasPanel").Hide();
    m_auiManager.GetPane("ImageViewer").Hide();
    m_auiManager.GetPane("EditorTabs").Show();
    m_auiManager.Update();

    auto editor = new EditorPanel(m_editorTabs, wxID_ANY);
    if (m_editorController != nullptr) {
        m_editorController->BindEditorEvents(editor);
    }
    m_editorTabs->AddPage(editor, "Untitled", true);
    auto* activePage = m_editorTabs->GetCurrentPage();
    if (activePage != nullptr) {
        m_documents[activePage] = Core::Document();
    }

    m_currentMode = ActivityMode::Notepad;
    m_activityBar->SetActiveMode(ActivityMode::Notepad);
    UpdateStatusBar();

    spdlog::info("MainFrame: opened new editor");
}

void MainFrame::OnOpen([[maybe_unused]] wxCommandEvent& event) {
    wxFileDialog openDialog(this, "Open File", "", "",
                            "All Supported Files|"
                            "*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.tif;*.webp;*.psd;*.psb;*.ico;*.xpm;"
                            "*.mp4;*.webm;*.mkv;*.avi;*.mov;*.mp3;*.wav;*.ogg;*.flac;"
                            "*.fbx;*.obj;*.gltf;*.glb;*.stl;*.dae;"
                            "*.txt;*.md;*.cpp;*.hpp;*.c;*.h;*.py;*.js;*.json;*.xml"
                            "|Image Files "
                            "(*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.webp;*.psd;*.psb)|*.png;*.jpg;*.jpeg;*.bmp;*.gif;"
                            "*.tiff;*.tif;*.webp;*.psd;*.psb"
                            "|Video Files (*.mp4;*.webm;*.mkv;*.avi;*.mov)|*.mp4;*.webm;*.mkv;*.avi;*.mov"
                            "|Audio Files (*.mp3;*.wav;*.ogg;*.flac)|*.mp3;*.wav;*.ogg;*.flac"
                            "|3D Models (*.fbx;*.obj;*.gltf;*.glb)|*.fbx;*.obj;*.gltf;*.glb"
                            "|Text Files (*.txt;*.md;*.cpp;*.hpp)|*.txt;*.md;*.cpp;*.hpp"
                            "|All Files (*.*)|*.*",
                            wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (openDialog.ShowModal() != wxID_OK)
        return;

    wxArrayString paths;
    openDialog.GetPaths(paths);
    for (const auto& wxPath : paths) {
        OpenPathUnified(std::filesystem::path(wxPath.ToStdString()));
    }
}

void MainFrame::OnOpenRecent(wxCommandEvent& event) {
    int index = event.GetId() - kOpenRecentBaseId;
    if (index < 0 || index >= static_cast<int>(m_recentFiles.size()))
        return;

    auto path = m_recentFiles[static_cast<size_t>(index)];
    if (!m_documentWorkflowService.PathExists(path)) {
        wxMessageBox("Recent file is no longer available: " + path.string(), "Open Recent", wxOK | wxICON_WARNING,
                     this);
        m_recentFiles.erase(m_recentFiles.begin() + index);
        RebuildOpenRecentMenu();
        return;
    }

    OpenPathUnified(path);
}

void MainFrame::OnQuickOpen([[maybe_unused]] wxCommandEvent& event) {
    wxTextEntryDialog dialog(this, "Enter part of a file path or a full path to open:", "Quick Open (Ctrl+P)");
    if (dialog.ShowModal() != wxID_OK)
        return;

    auto query = dialog.GetValue().ToStdString();
    if (query.empty())
        return;

    std::filesystem::path directPath(query);
    if (m_documentWorkflowService.PathExists(directPath)) {
        OpenPathUnified(directPath);
        return;
    }

    auto lower = query;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    for (const auto& recentPath : m_recentFiles) {
        auto recentLower = recentPath.string();
        std::transform(recentLower.begin(), recentLower.end(), recentLower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (recentLower.find(lower) != std::string::npos) {
            OpenPathUnified(recentPath);
            return;
        }
    }

    wxMessageBox("No matching file found in recent items and path does not exist.", "Quick Open",
                 wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnSave([[maybe_unused]] wxCommandEvent& event) {
    auto* page = m_editorTabs != nullptr ? m_editorTabs->GetCurrentPage() : nullptr;
    if (page == nullptr) {
        wxMessageBox("No active text editor tab to save.", "Save", wxOK | wxICON_INFORMATION, this);
        return;
    }

    auto saveResult = m_editorController != nullptr
                          ? m_editorController->SaveDocumentForPage(page, false, [this]() { UpdateStatusBar(); })
                          : std::unexpected(std::string("Editor controller is unavailable."));
    if (!saveResult) {
        if (IsSaveCancelled(saveResult.error())) {
            return;
        }
        wxMessageBox(saveResult.error(), "Save Error", wxOK | wxICON_ERROR, this);
        return;
    }
}

void MainFrame::OnSaveAs([[maybe_unused]] wxCommandEvent& event) {
    auto* page = m_editorTabs != nullptr ? m_editorTabs->GetCurrentPage() : nullptr;
    if (page == nullptr) {
        wxMessageBox("No active text editor tab to save.", "Save As", wxOK | wxICON_INFORMATION, this);
        return;
    }

    auto saveResult = m_editorController != nullptr
                          ? m_editorController->SaveDocumentForPage(page, true, [this]() { UpdateStatusBar(); })
                          : std::unexpected(std::string("Editor controller is unavailable."));
    if (!saveResult) {
        if (IsSaveCancelled(saveResult.error())) {
            return;
        }
        wxMessageBox(saveResult.error(), "Save As Error", wxOK | wxICON_ERROR, this);
        return;
    }
}

void MainFrame::OnDeleteFile([[maybe_unused]] wxCommandEvent& event) {
    auto* page = m_editorTabs != nullptr ? m_editorTabs->GetCurrentPage() : nullptr;
    if (page == nullptr) {
        wxMessageBox("No active text editor tab to delete from disk.", "Delete File", wxOK | wxICON_INFORMATION, this);
        return;
    }

    auto docIt = m_documents.find(page);
    if (docIt == m_documents.end() || !docIt->second.GetFilePath().has_value()) {
        wxMessageBox("Current document has no saved file path.", "Delete File", wxOK | wxICON_INFORMATION, this);
        return;
    }

    const auto filePath = docIt->second.GetFilePath();
    auto path = *filePath;
    if (docIt->second.IsModified()) {
        auto saveAnswer = wxMessageBox("This document has unsaved changes. Save before deleting the file?",
                                       "Unsaved Changes", wxYES_NO | wxCANCEL | wxICON_WARNING, this);
        if (saveAnswer == wxCANCEL) {
            return;
        }
        if (saveAnswer == wxYES) {
            auto saveResult = m_editorController != nullptr
                                  ? m_editorController->SaveDocumentForPage(page, false, [] {})
                                  : std::unexpected(std::string("Editor controller is unavailable."));
            if (!saveResult) {
                if (!IsSaveCancelled(saveResult.error())) {
                    wxMessageBox(saveResult.error(), "Save Error", wxOK | wxICON_ERROR, this);
                }
                return;
            }
        }
    }

    auto confirm = wxMessageBox("Delete this file from disk?\n\n" + path.string(), "Delete File",
                                wxYES_NO | wxCANCEL | wxICON_WARNING, this);
    if (confirm != wxYES) {
        return;
    }

    auto deleteResult = m_documentWorkflowService.DeleteDocumentFile(path);
    if (!deleteResult) {
        wxMessageBox(deleteResult.error(), "Delete Error", wxOK | wxICON_ERROR, this);
        return;
    }

    auto index = m_editorTabs->GetPageIndex(page);
    if (index != wxNOT_FOUND) {
        m_editorTabs->DeletePage(static_cast<size_t>(index));
    }
    spdlog::info("MainFrame: deleted file {}", path.string());
}

void MainFrame::OnExit([[maybe_unused]] wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnUpdateSaveUi(wxUpdateUIEvent& event) {
    auto* page = m_editorTabs != nullptr ? m_editorTabs->GetCurrentPage() : nullptr;
    auto* editor = dynamic_cast<EditorPanel*>(page);
    event.Enable(editor != nullptr && m_editorController != nullptr);
}

void MainFrame::OnUpdateSaveAsUi(wxUpdateUIEvent& event) {
    auto* page = m_editorTabs != nullptr ? m_editorTabs->GetCurrentPage() : nullptr;
    auto* editor = dynamic_cast<EditorPanel*>(page);
    event.Enable(editor != nullptr && m_editorController != nullptr);
}

void MainFrame::OnUpdateDeleteFileUi(wxUpdateUIEvent& event) {
    auto* page = m_editorTabs != nullptr ? m_editorTabs->GetCurrentPage() : nullptr;
    auto* editor = dynamic_cast<EditorPanel*>(page);
    if (editor == nullptr) {
        event.Enable(false);
        return;
    }
    auto docIt = m_documents.find(page);
    event.Enable(docIt != m_documents.end() && docIt->second.GetFilePath().has_value());
}

void MainFrame::OnEditorTabClosed(wxAuiNotebookEvent& event) {
    if (m_editorTabs == nullptr) {
        event.Skip();
        return;
    }

    auto pageIndex = static_cast<size_t>(event.GetSelection());
    if (pageIndex >= m_editorTabs->GetPageCount()) {
        event.Skip();
        return;
    }

    auto* page = m_editorTabs->GetPage(pageIndex);
    if (m_editorController != nullptr && !m_editorController->ConfirmClosePage(page)) {
        event.Veto();
        return;
    }

    if (page != nullptr) {
        m_documents.erase(page);
    }

    event.Skip();
}

void MainFrame::OnAbout([[maybe_unused]] wxCommandEvent& event) {
    wxMessageDialog dlg(this,
                        wxString::Format("CLIADE AI Content Creator\nVersion v%s\n\n"
                                         "AJC-Software Ltd \xC2\xA9 2026\n\n"
                                         "Cross-platform AIO IDE for code editing, media "
                                         "workflows, and AI-powered content generation.",
                                         CLIADE_VERSION_STRING),
                        "About CLIADE", wxOK | wxICON_INFORMATION);
    dlg.ShowModal();
}

void MainFrame::OnRuntimeDiagnostics([[maybe_unused]] wxCommandEvent& event) {
    const char* availability = m_vulkanRuntime.IsVulkanAvailable() ? "Available" : "Unavailable";
    const char* loaded = m_vulkanRuntime.IsLoaded() ? "Loaded" : "Not loaded";
    const char* initialized = m_vulkanRuntime.IsInitialized() ? "Yes" : "No";
    const char* api = m_vulkanRuntime.IsApiCompatible() ? "Compatible" : "Not compatible";
    auto availabilityReason = m_vulkanRuntime.GetAvailabilityReason();
    auto availabilityReasonCode = m_vulkanRuntime.GetAvailabilityReasonCode();
    auto availabilityReasonCodeName = wxString::FromUTF8(AvailabilityReasonCodeName(availabilityReasonCode));
    auto installHelpUrl = m_vulkanRuntime.GetInstallHelpUrl();
    auto searchPaths = m_vulkanRuntime.GetRuntimeSearchPaths();
    auto loadAttempts = m_vulkanRuntime.GetLastLoadAttempts();
    auto caps = m_vulkanRuntime.GetCapabilities();
    auto reason = wxString::FromUTF8(availabilityReason.c_str());
    auto helpUrl = wxString::FromUTF8(installHelpUrl.c_str());
    wxString attemptsText = "(none)";
    if (!loadAttempts.empty()) {
        attemptsText.clear();
        for (std::size_t i = 0; i < loadAttempts.size(); ++i) {
            attemptsText += wxString::Format("%zu) %s", i + 1, loadAttempts[i]);
            if (i + 1 < loadAttempts.size()) {
                attemptsText += "\n";
            }
        }
    }
    wxString searchPathsText = "(none)";
    if (!searchPaths.empty()) {
        searchPathsText.clear();
        for (std::size_t i = 0; i < searchPaths.size(); ++i) {
            searchPathsText += wxString::Format("%zu) %s", i + 1, searchPaths[i]);
            if (i + 1 < searchPaths.size()) {
                searchPathsText += "\n";
            }
        }
    }

    auto message = wxString::Format("Vulkan runtime status\n\n"
                                    "Module: %s\n"
                                    "Initialized: %s\n"
                                    "API: %s\n"
                                    "Runtime API Version: %u.%u\n"
                                    "Runtime Version: %u.%u.%u\n"
                                    "Vulkan: %s\n"
                                    "Reason Code: %u\n"
                                    "Reason Name: %s\n"
                                    "Details: %s\n"
                                    "Search paths:\n%s\n"
                                    "Load attempts:\n%s\n"
                                    "Install help: %s",
                                    loaded, initialized, api, caps.apiVersion.major, caps.apiVersion.minor,
                                    caps.runtimeVersion.major, caps.runtimeVersion.minor, caps.runtimeVersion.patch,
                                    availability, static_cast<unsigned>(availabilityReasonCode),
                                    availabilityReasonCodeName, reason, searchPathsText, attemptsText, helpUrl);

    wxMessageDialog dlg(this, message, "Runtime Diagnostics", wxOK | wxICON_INFORMATION);
    dlg.ShowModal();
}

void MainFrame::OnRetryRuntime([[maybe_unused]] wxCommandEvent& event) {
    spdlog::info("MainFrame: retrying Vulkan runtime initialization");
    ResetVulkanRuntimeState();
    InitializeVulkanRuntime();
    UpdateStatusBar();

    if (m_vulkanRenderHost == nullptr) {
        auto message = wxString::Format("Vulkan runtime retry did not succeed.\n\nStatus: %s\nError: %s",
                                        m_vulkanStatus, m_vulkanRuntime.GetLastError());
        wxMessageBox(message, "Vulkan Runtime Retry", wxOK | wxICON_WARNING, this);
    }
}

void MainFrame::OnClose(wxCloseEvent& event) {
    if (m_editorTabs != nullptr && m_editorController != nullptr) {
        for (size_t i = 0; i < m_editorTabs->GetPageCount(); ++i) {
            auto* page = m_editorTabs->GetPage(i);
            if (!m_editorController->ConfirmClosePage(page)) {
                spdlog::info("MainFrame: close cancelled by unsaved tab");
                event.Veto();
                return;
            }
        }
    }

    ResetVulkanRuntimeState();
    m_auiManager.UnInit();
    spdlog::info("MainFrame: closing");
    event.Skip();
}

} // namespace Ui

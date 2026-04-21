// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MainFrame.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/aui/aui.h>
#include <wx/bitmap.h>
#include <wx/frame.h>
#include <wx/panel.h>

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "core/Document.hpp"
#include "core/DocumentWorkflowService.hpp"
#include "core/VulkanRuntimeLoader.hpp"
#include "core/runtime/VulkanRenderHost.hpp"
#include "ui/ActivityBar.hpp"

namespace Ui {

class BackgroundPanel;
class CanvasPanel;
class EditorDocumentController;
class EditorPanel;
class ImageViewer;
class PropertiesPanel;
class PromptBar;

class MainFrame : public wxFrame
{
  public:
    MainFrame();
    void OpenDroppedFile(const std::filesystem::path& path);

  private:
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnOpenRecent(wxCommandEvent& event);
    void OnQuickOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnDeleteFile(wxCommandEvent& event);
    void OnUpdateSaveUi(wxUpdateUIEvent& event);
    void OnUpdateSaveAsUi(wxUpdateUIEvent& event);
    void OnUpdateDeleteFileUi(wxUpdateUIEvent& event);
    void OnRuntimeDiagnostics(wxCommandEvent& event);
    void OnRetryRuntime(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void CreateMenuBar();
    void CreateDockingSystem();
    void LoadBackgroundImage();
    void LoadAppIcon();
    void OnActivityModeChanged(ActivityMode mode);
    void OpenImage(const std::filesystem::path& path);
    void OpenTextFile(const std::filesystem::path& path);
    void OpenPathUnified(const std::filesystem::path& path, bool addToRecent = true);
    void AddRecentFile(const std::filesystem::path& path);
    void RebuildOpenRecentMenu();
    void OnEditorTabClosed(wxAuiNotebookEvent& event);
    void InitializeVulkanRuntime();
    void ResetVulkanRuntimeState();
    void OnWindowResized(wxSizeEvent& event);
    void OnIdle(wxIdleEvent& event);
    void HandleVulkanFrameFailure(std::string_view reason);
    void UpdateStatusBar();

    wxAuiManager m_auiManager;
    wxAuiNotebook* m_editorTabs;
    ImageViewer* m_imageViewer;
    CanvasPanel* m_canvasPanel;
    BackgroundPanel* m_bgPanel;
    ActivityBar* m_activityBar;
    PropertiesPanel* m_propertiesPanel;
    PromptBar* m_promptBar;
    std::unique_ptr<EditorDocumentController> m_editorController;
    std::unordered_map<wxWindow*, Core::Document> m_documents;
    wxMenu* m_openRecentMenu;
    std::deque<std::filesystem::path> m_recentFiles;
    ActivityMode m_currentMode;
    Core::DocumentWorkflowService m_documentWorkflowService;
    Core::VulkanRuntimeLoader m_vulkanRuntime;
    std::unique_ptr<Core::VulkanRenderHost> m_vulkanRenderHost;
    std::string m_vulkanStatus;
    bool m_vulkanFrameLoopDisabled;
};

} // namespace Ui

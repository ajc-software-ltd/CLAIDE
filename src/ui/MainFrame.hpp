// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        MainFrame.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/frame.h>
#include <wx/bitmap.h>
#include <wx/panel.h>
#include <wx/aui/aui.h>

#include <functional>
#include <deque>
#include <unordered_map>

#include "core/Document.hpp"
#include "ui/ActivityBar.hpp"

namespace Ui {

class BackgroundPanel;
class ImageViewer;
class PropertiesPanel;
class PromptBar;

class MainFrame : public wxFrame {
public:
    MainFrame();
    void OpenDroppedFile(const std::filesystem::path& path);

private:
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnOpenRecent(wxCommandEvent& event);
    void OnQuickOpen(wxCommandEvent& event);
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
    void UpdateStatusBar();

    wxAuiManager m_auiManager;
    wxAuiNotebook* m_editorTabs;
    ImageViewer* m_imageViewer;
    BackgroundPanel* m_bgPanel;
    ActivityBar* m_activityBar;
    PropertiesPanel* m_propertiesPanel;
    PromptBar* m_promptBar;
    std::unordered_map<wxWindow*, Core::Document> m_documents;
    wxMenu* m_openRecentMenu;
    std::deque<std::filesystem::path> m_recentFiles;
    ActivityMode m_currentMode;
};

} // namespace Ui

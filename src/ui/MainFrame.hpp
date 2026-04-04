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
#include <wx/generic/dirctrlg.h>

#include <map>
#include <functional>

#include "core/Document.hpp"
#include "core/MediaService.hpp"

namespace Ui {

class ImageViewer;

class BackgroundPanel : public wxPanel {
public:
    BackgroundPanel(wxWindow* parent);

    void SetBackgroundBitmap(const wxBitmap& bmp);

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);

    wxBitmap m_bitmap;
    wxBitmap m_scaledBitmap;
};

enum class SidebarMode {
    Files,
    Images,
    Video,
    Models
};

class IconBar : public wxPanel {
public:
    IconBar(wxWindow* parent);

    void SetModeCallback(std::function<void(SidebarMode)> cb) { m_modeCb = std::move(cb); }

private:
    void OnMouse(wxMouseEvent& event);
    void OnPaint(wxPaintEvent& event);

    struct IconEntry {
        wxBitmap bitmap;
        wxBitmap activeBitmap;
        wxBitmap inactiveBitmap;
        wxString label;
        SidebarMode mode;
        wxRect hitRect;
    };

    std::vector<IconEntry> m_icons;
    SidebarMode m_activeMode;
    std::function<void(SidebarMode)> m_modeCb;
};

class FileExplorerPanel : public wxPanel {
public:
    FileExplorerPanel(wxWindow* parent);

    void LoadDirectory(const std::string& path);
    void SetFileOpenCallback(std::function<void(const std::string&)> cb) {
        m_fileOpenCb = std::move(cb);
    }

private:
    void OnOpenFolder(wxCommandEvent& event);
    void OnDetach(wxCommandEvent& event);
    void OnFileActivated(wxTreeEvent& event);

    wxGenericDirCtrl* m_dirCtrl;
    wxButton* m_openFolderBtn;
    wxButton* m_detachBtn;
    std::function<void(const std::string&)> m_fileOpenCb;
};

class PromptBar : public wxPanel {
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

class MainFrame : public wxFrame {
public:
    MainFrame();

private:
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void CreateMenuBar();
    void CreateDockingSystem();
    void LoadBackgroundImage();
    void LoadAppIcon();
    void SetSidebarMode(SidebarMode mode);
    void OpenImage(const std::filesystem::path& path);
    void OpenTextFile(const std::filesystem::path& path);
    void UpdateStatusBar();

    wxAuiManager m_auiManager;
    wxAuiNotebook* m_editorTabs;
    ImageViewer* m_imageViewer;
    BackgroundPanel* m_bgPanel;
    IconBar* m_iconBar;
    FileExplorerPanel* m_fileExplorer;
    PromptBar* m_promptBar;
    std::map<size_t, Core::Document> m_documents;
    SidebarMode m_currentMode;
};

} // namespace Ui

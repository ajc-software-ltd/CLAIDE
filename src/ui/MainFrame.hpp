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

#include "core/Document.hpp"

namespace Ui {

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

class IconBar : public wxPanel {
public:
    IconBar(wxWindow* parent);

    void SetFolderCallback(std::function<void()> cb) { m_folderCb = std::move(cb); }

private:
    void OnFolder(wxMouseEvent& event);
    void OnPaint(wxPaintEvent& event);

    std::function<void()> m_folderCb;
    wxBitmap m_folderBmp;
    bool m_folderActive;
};

class FileExplorerPanel : public wxPanel {
public:
    FileExplorerPanel(wxWindow* parent);

    void LoadDirectory(const std::string& path);

private:
    void OnOpenFolder(wxCommandEvent& event);
    void OnDetach(wxCommandEvent& event);

    wxGenericDirCtrl* m_dirCtrl;
    wxButton* m_openFolderBtn;
    wxButton* m_detachBtn;
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
    void ShowFileExplorer();
    void HideFileExplorer();
    void CreateStatusBar();

    wxAuiManager m_auiManager;
    wxAuiNotebook* m_editorTabs;
    BackgroundPanel* m_bgPanel;
    IconBar* m_iconBar;
    FileExplorerPanel* m_fileExplorer;
    PromptBar* m_promptBar;
    std::map<size_t, Core::Document> m_documents;
    bool m_explorerVisible;
};

} // namespace Ui

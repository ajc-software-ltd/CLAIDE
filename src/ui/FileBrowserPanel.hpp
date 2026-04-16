// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileBrowserPanel.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/choice.h>

#include <filesystem>
#include <functional>
#include <vector>

#include "core/FileBrowserService.hpp"
#include "ui/ThumbnailCache.hpp"

namespace Ui {

class FileBrowserPanel : public wxPanel {
public:
    FileBrowserPanel(wxWindow* parent);

    void SetFileOpenCallback(std::function<void(const std::string&)> cb) {
        m_fileOpenCb = std::move(cb);
    }

    void NavigateTo(const std::filesystem::path& path);
    void GoBack();
    void GoForward();
    void GoUp();

private:
    void OnBack(wxCommandEvent& event);
    void OnForward(wxCommandEvent& event);
    void OnUp(wxCommandEvent& event);
    void OnFilterChanged(wxCommandEvent& event);
    void OnPathEntered(wxCommandEvent& event);
    void OnGridPaint(wxPaintEvent& event);
    void OnGridScroll(wxScrollWinEvent& event);
    void OnGridSize(wxSizeEvent& event);
    void OnGridLeftDClick(wxMouseEvent& event);

    void LoadDirectory(std::filesystem::path path);
    void UpdateLayout();
    void RenderGrid(wxDC& dc);
    int GetIndexAtPosition(wxPoint pos) const;
    wxPanel* m_toolbar;
    wxScrolledWindow* m_grid;
    wxTextCtrl* m_pathBar;
    wxButton* m_backBtn;
    wxButton* m_forwardBtn;
    wxButton* m_upBtn;
    wxChoice* m_filterChoice;

    std::filesystem::path m_currentPath;
    std::filesystem::path m_homePath;
    std::vector<Core::FileBrowserEntry> m_items;
    std::vector<std::filesystem::path> m_backHistory;
    std::vector<std::filesystem::path> m_forwardHistory;

    ThumbnailCache m_thumbnailCache;

    int m_cellSize = 160;
    int m_rowHeight = 190;
    int m_columns = 4;
    int m_filterIndex = 0;

    std::function<void(const std::string&)> m_fileOpenCb;
};

} // namespace Ui

// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileBrowserPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/FileBrowserPanel.hpp"

#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/renderer.h>
#include <wx/sizer.h>

#include <algorithm>
#include <cstdlib>

#include <spdlog/spdlog.h>

namespace Ui {

FileBrowserPanel::FileBrowserPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_toolbar(nullptr), m_grid(nullptr), m_pathBar(nullptr), m_backBtn(nullptr),
      m_forwardBtn(nullptr), m_upBtn(nullptr), m_filterChoice(nullptr), m_filterIndex(0) {
    SetBackgroundColour(wxColour(30, 30, 30));

    const char* home = nullptr;
#ifdef _WIN32
    home = std::getenv("USERPROFILE");
#else
    home = std::getenv("HOME");
#endif
    m_homePath = home ? std::filesystem::path(home) : std::filesystem::current_path();
    m_currentPath = m_homePath;

    // Toolbar
    m_toolbar = new wxPanel(this, wxID_ANY);
    m_toolbar->SetBackgroundColour(wxColour(50, 50, 50));
    m_toolbar->SetMinSize(wxSize(-1, 40));

    auto toolbarSizer = new wxBoxSizer(wxHORIZONTAL);

    m_backBtn = new wxButton(m_toolbar, wxID_ANY, "Back", wxDefaultPosition, wxSize(60, 28));
    m_backBtn->SetBackgroundColour(wxColour(60, 60, 60));
    m_backBtn->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_backBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    m_forwardBtn = new wxButton(m_toolbar, wxID_ANY, "Forward", wxDefaultPosition, wxSize(60, 28));
    m_forwardBtn->SetBackgroundColour(wxColour(60, 60, 60));
    m_forwardBtn->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_forwardBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    m_upBtn = new wxButton(m_toolbar, wxID_ANY, "Up", wxDefaultPosition, wxSize(50, 28));
    m_upBtn->SetBackgroundColour(wxColour(60, 60, 60));
    m_upBtn->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_upBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    m_pathBar = new wxTextCtrl(m_toolbar, wxID_ANY, m_currentPath.string(), wxDefaultPosition, wxDefaultSize,
                               wxTE_PROCESS_ENTER);
    m_pathBar->SetBackgroundColour(wxColour(40, 40, 40));
    m_pathBar->SetForegroundColour(wxColour(220, 220, 220));
    toolbarSizer->Add(m_pathBar, 1, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    m_filterChoice = new wxChoice(m_toolbar, wxID_ANY);
    m_filterChoice->Append("All Files");
    m_filterChoice->Append("Images");
    m_filterChoice->Append("Video");
    m_filterChoice->Append("3D Models");
    m_filterChoice->SetSelection(0);
    m_filterChoice->SetBackgroundColour(wxColour(60, 60, 60));
    m_filterChoice->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_filterChoice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4);

    m_toolbar->SetSizer(toolbarSizer);

    // Grid (scrollable area)
    m_grid = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    m_grid->SetBackgroundColour(wxColour(30, 30, 30));
    m_grid->SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_grid->SetScrollRate(20, 20);

    m_grid->Bind(wxEVT_PAINT, &FileBrowserPanel::OnGridPaint, this);
    m_grid->Bind(wxEVT_SIZE, &FileBrowserPanel::OnGridSize, this);
    m_grid->Bind(wxEVT_LEFT_DCLICK, &FileBrowserPanel::OnGridLeftDClick, this);
    m_grid->Bind(wxEVT_SCROLLWIN_TOP, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_BOTTOM, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_LINEUP, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_LINEDOWN, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_PAGEUP, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_PAGEDOWN, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_THUMBTRACK, &FileBrowserPanel::OnGridScroll, this);
    m_grid->Bind(wxEVT_SCROLLWIN_THUMBRELEASE, &FileBrowserPanel::OnGridScroll, this);

    m_backBtn->Bind(wxEVT_BUTTON, &FileBrowserPanel::OnBack, this);
    m_forwardBtn->Bind(wxEVT_BUTTON, &FileBrowserPanel::OnForward, this);
    m_upBtn->Bind(wxEVT_BUTTON, &FileBrowserPanel::OnUp, this);
    m_filterChoice->Bind(wxEVT_CHOICE, &FileBrowserPanel::OnFilterChanged, this);
    m_pathBar->Bind(wxEVT_TEXT_ENTER, &FileBrowserPanel::OnPathEntered, this);

    // Layout
    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_toolbar, 0, wxEXPAND);
    mainSizer->Add(m_grid, 1, wxEXPAND);
    SetSizer(mainSizer);

    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::NavigateTo(const std::filesystem::path& path) {
    try {
        spdlog::info("FileBrowserPanel: navigating to {}", path.string());

        if (!Core::FileBrowserService::IsDirectoryPath(path)) {
            spdlog::warn("FileBrowserPanel: path does not exist: {}", path.string());
            return;
        }

        m_backHistory.push_back(m_currentPath);
        m_forwardHistory.clear();
        m_currentPath = path;

        spdlog::info("FileBrowserPanel: loading directory {}", path.string());
        LoadDirectory(path);
        spdlog::info("FileBrowserPanel: navigation complete");
    } catch (const std::exception& e) {
        spdlog::error("FileBrowserPanel: navigation failed: {}", e.what());
    }
}

void FileBrowserPanel::GoBack() {
    if (m_backHistory.empty()) {
        return;
    }

    m_forwardHistory.push_back(m_currentPath);
    m_currentPath = m_backHistory.back();
    m_backHistory.pop_back();
    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::GoForward() {
    if (m_forwardHistory.empty()) {
        return;
    }

    m_backHistory.push_back(m_currentPath);
    m_currentPath = m_forwardHistory.back();
    m_forwardHistory.pop_back();
    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::GoUp() {
    if (m_currentPath == m_homePath) {
        return;
    }

    auto parent = m_currentPath.parent_path();
    if (!parent.empty() && parent != m_currentPath) {
        NavigateTo(parent);
    }
}

void FileBrowserPanel::LoadDirectory(std::filesystem::path path) {
    auto filter = static_cast<Core::FileBrowserFilter>(m_filterIndex);
    auto itemsResult = Core::FileBrowserService::ListDirectory(path, filter);
    if (!itemsResult) {
        m_items.clear();
        spdlog::warn("FileBrowserPanel: failed to read {}: {}", path.string(), itemsResult.error());
        return;
    }
    m_items = std::move(*itemsResult);

    if (m_pathBar) {
        m_pathBar->SetValue(path.string());
    }

    UpdateLayout();
    m_grid->Refresh();
}

void FileBrowserPanel::UpdateLayout() {
    wxSize clientSize = m_grid->GetClientSize();
    if (clientSize.GetWidth() <= 0) {
        return;
    }

    int availableWidth = clientSize.GetWidth() - 8;
    m_columns = std::max(1, availableWidth / m_cellSize);
    int rows = (m_items.empty()) ? 1 : (static_cast<int>(m_items.size()) + m_columns - 1) / m_columns;

    int totalHeight = rows * m_rowHeight + 60;

    // Only update if size actually changed
    int currentW = 0, currentH = 0;
    m_grid->GetVirtualSize(&currentW, &currentH);
    if (currentW != availableWidth || currentH != totalHeight) {
        m_grid->SetVirtualSize(availableWidth, totalHeight);
    }
}

void FileBrowserPanel::OnGridPaint([[maybe_unused]] wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(m_grid);
    PrepareDC(dc);
    dc.SetBackground(wxBrush(wxColour(30, 30, 30)));
    dc.Clear();

    int startX = 4;
    int startY = 4;
    int iconSize = 128;

    for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
        int col = i % m_columns;
        int row = i / m_columns;

        int x = startX + col * m_cellSize;
        int y = startY + row * m_rowHeight;

        auto& item = m_items[i].path;

        wxBitmap thumb;
        try {
            thumb = m_thumbnailCache.GetThumbnail(item, iconSize);
        } catch (const std::exception& e) {
            spdlog::warn("FileBrowserPanel: thumbnail error for {}: {}", item.filename().string(), e.what());
        }

        if (thumb.IsOk()) {
            int tx = x + (m_cellSize - iconSize) / 2;
            int ty = y;
            dc.DrawBitmap(thumb, tx, ty, true);
        }

        wxString name = item.filename().string();
        if (name.Len() > 16) {
            name = name.SubString(0, 13) + "...";
        }

        dc.SetTextForeground(wxColour(200, 200, 200));
        dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

        wxCoord textW = 0, textH = 0;
        dc.GetTextExtent(name, &textW, &textH);

        int textX = x + (m_cellSize - textW) / 2;
        int textY = y + iconSize + 4;

        dc.DrawText(name, textX, textY);
    }
}

void FileBrowserPanel::OnGridScroll(wxScrollWinEvent& event) {
    m_grid->Refresh();
    event.Skip();
}

void FileBrowserPanel::OnGridSize(wxSizeEvent& event) {
    UpdateLayout();
    event.Skip();
}

void FileBrowserPanel::OnGridLeftDClick(wxMouseEvent& event) {
    try {
        wxPoint pos = event.GetPosition();
        int viewStartX = 0, viewStartY = 0;
        m_grid->GetViewStart(&viewStartX, &viewStartY);
        int scrollPxX = 0, scrollPxY = 0;
        m_grid->GetScrollPixelsPerUnit(&scrollPxX, &scrollPxY);

        int adjX = pos.x + viewStartX * scrollPxX;
        int adjY = pos.y + viewStartY * scrollPxY;

        int startX = 4;
        int startY = 4;

        int col = (adjX - startX) / m_cellSize;
        int row = (adjY - startY) / m_rowHeight;

        if (col < 0 || col >= m_columns || row < 0)
            return;

        int index = row * m_columns + col;
        if (index < 0 || index >= static_cast<int>(m_items.size()))
            return;

        const auto& item = m_items[index];
        spdlog::info("FileBrowserPanel: double-clicked {}", item.path.string());

        if (item.isDirectory) {
            NavigateTo(item.path);
        } else if (m_fileOpenCb) {
            m_fileOpenCb(item.path.string());
        }
    } catch (const std::exception& e) {
        spdlog::error("FileBrowserPanel: double-click handler failed: {}", e.what());
    }

    event.Skip();
}

void FileBrowserPanel::OnBack(wxCommandEvent& event) {
    (void)event;
    GoBack();
}

void FileBrowserPanel::OnForward(wxCommandEvent& event) {
    (void)event;
    GoForward();
}

void FileBrowserPanel::OnUp(wxCommandEvent& event) {
    (void)event;
    GoUp();
}

void FileBrowserPanel::OnFilterChanged(wxCommandEvent& event) {
    (void)event;
    m_filterIndex = m_filterChoice->GetSelection();
    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::OnPathEntered(wxCommandEvent& event) {
    (void)event;
    auto path = m_pathBar->GetValue().ToStdString();
    if (Core::FileBrowserService::IsDirectoryPath(path)) {
        NavigateTo(path);
    }
}

} // namespace Ui

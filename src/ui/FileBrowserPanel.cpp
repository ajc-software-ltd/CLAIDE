// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileBrowserPanel.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/FileBrowserPanel.hpp"

#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/renderer.h>

#include <algorithm>
#include <cstdlib>

#include <spdlog/spdlog.h>

#include "core/MediaService.hpp"

namespace Ui {

FileBrowserPanel::FileBrowserPanel(wxWindow* parent)
    : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                       wxHSCROLL | wxVSCROLL),
      m_pathBar(nullptr), m_backBtn(nullptr), m_forwardBtn(nullptr),
      m_upBtn(nullptr), m_filterChoice(nullptr), m_filterIndex(0) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    auto home = std::getenv("HOME");
    m_homePath = home ? std::filesystem::path(home) : std::filesystem::current_path();
    m_currentPath = m_homePath;

    auto toolbar = new wxPanel(this, wxID_ANY);
    toolbar->SetBackgroundColour(wxColour(50, 50, 50));
    auto toolbarSizer = new wxBoxSizer(wxHORIZONTAL);

    m_backBtn = new wxButton(toolbar, wxID_ANY, "←",
                             wxDefaultPosition, wxSize(32, 28));
    m_backBtn->SetBackgroundColour(wxColour(60, 60, 60));
    m_backBtn->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_backBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_forwardBtn = new wxButton(toolbar, wxID_ANY, "→",
                                wxDefaultPosition, wxSize(32, 28));
    m_forwardBtn->SetBackgroundColour(wxColour(60, 60, 60));
    m_forwardBtn->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_forwardBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_upBtn = new wxButton(toolbar, wxID_ANY, "↑",
                           wxDefaultPosition, wxSize(32, 28));
    m_upBtn->SetBackgroundColour(wxColour(60, 60, 60));
    m_upBtn->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_upBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_pathBar = new wxTextCtrl(toolbar, wxID_ANY, m_currentPath.string(),
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_PROCESS_ENTER);
    m_pathBar->SetBackgroundColour(wxColour(40, 40, 40));
    m_pathBar->SetForegroundColour(wxColour(220, 220, 220));
    toolbarSizer->Add(m_pathBar, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    m_filterChoice = new wxChoice(toolbar, wxID_ANY);
    m_filterChoice->Append("All Files");
    m_filterChoice->Append("Images");
    m_filterChoice->Append("Video");
    m_filterChoice->Append("3D Models");
    m_filterChoice->SetSelection(0);
    m_filterChoice->SetBackgroundColour(wxColour(60, 60, 60));
    m_filterChoice->SetForegroundColour(wxColour(200, 200, 200));
    toolbarSizer->Add(m_filterChoice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    toolbar->SetSizer(toolbarSizer);

    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(toolbar, 0, wxEXPAND);
    SetSizer(mainSizer);

    Bind(wxEVT_PAINT, &FileBrowserPanel::OnPaint, this);
    Bind(wxEVT_SIZE, &FileBrowserPanel::OnSize, this);
    Bind(wxEVT_LEFT_DCLICK, &FileBrowserPanel::OnLeftDClick, this);
    m_backBtn->Bind(wxEVT_BUTTON, &FileBrowserPanel::OnBack, this);
    m_forwardBtn->Bind(wxEVT_BUTTON, &FileBrowserPanel::OnForward, this);
    m_upBtn->Bind(wxEVT_BUTTON, &FileBrowserPanel::OnUp, this);
    m_filterChoice->Bind(wxEVT_CHOICE, &FileBrowserPanel::OnFilterChanged, this);
    m_pathBar->Bind(wxEVT_TEXT_ENTER, &FileBrowserPanel::OnPathEntered, this);

    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::NavigateTo(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        return;
    }

    m_backHistory.push_back(m_currentPath);
    m_forwardHistory.clear();
    m_currentPath = path;
    LoadDirectory(path);
}

void FileBrowserPanel::GoBack() {
    if (m_backHistory.empty()) return;

    m_forwardHistory.push_back(m_currentPath);
    m_currentPath = m_backHistory.back();
    m_backHistory.pop_back();
    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::GoForward() {
    if (m_forwardHistory.empty()) return;

    m_backHistory.push_back(m_currentPath);
    m_currentPath = m_forwardHistory.back();
    m_forwardHistory.pop_back();
    LoadDirectory(m_currentPath);
}

void FileBrowserPanel::GoUp() {
    if (m_currentPath == m_homePath) return;

    auto parent = m_currentPath.parent_path();
    if (!parent.empty() && parent != m_currentPath) {
        NavigateTo(parent);
    }
}

void FileBrowserPanel::LoadDirectory(const std::filesystem::path& path) {
    m_items.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            const auto& p = entry.path();
            auto name = p.filename().string();

            // Skip hidden files
            if (!name.empty() && name[0] == '.') continue;

            if (entry.is_directory()) {
                m_items.push_back(p);
            } else if (entry.is_regular_file() && MatchesFilter(p)) {
                m_items.push_back(p);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        spdlog::warn("FileBrowserPanel: failed to read {}: {}",
                     path.string(), e.what());
    }

    std::sort(m_items.begin(), m_items.end(),
              [](const std::filesystem::path& a, const std::filesystem::path& b) {
                  bool aDir = std::filesystem::is_directory(a);
                  bool bDir = std::filesystem::is_directory(b);
                  if (aDir != bDir) return aDir;
                  return a.filename().string() < b.filename().string();
              });

    if (m_pathBar) {
        m_pathBar->SetValue(path.string());
    }

    UpdateLayout();
    Refresh();

    spdlog::debug("FileBrowserPanel: loaded {} items from {}",
                  m_items.size(), path.string());
}

void FileBrowserPanel::UpdateLayout() {
    wxSize clientSize = GetClientSize();
    int toolbarHeight = 40;
    int availableWidth = clientSize.GetWidth() - 8;
    (void)toolbarHeight;

    m_columns = std::max(1, availableWidth / m_cellSize);
    int rows = (m_items.empty()) ? 1 :
               (static_cast<int>(m_items.size()) + m_columns - 1) / m_columns;

    SetVirtualSize(availableWidth, rows * m_rowHeight + 10);
    SetScrollRate(1, 1);
}

void FileBrowserPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);
    RenderGrid(dc);
    event.Skip();
}

void FileBrowserPanel::OnSize(wxSizeEvent& event) {
    UpdateLayout();
    Refresh();
    event.Skip();
}

void FileBrowserPanel::OnLeftDClick(wxMouseEvent& event) {
    int index = GetIndexAtPosition(event.GetPosition());
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        auto& item = m_items[index];

        if (std::filesystem::is_directory(item)) {
            NavigateTo(item);
        } else if (m_fileOpenCb) {
            m_fileOpenCb(item.string());
        }
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
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        NavigateTo(path);
    }
}

void FileBrowserPanel::RenderGrid(wxDC& dc) {
    dc.SetBackground(wxBrush(wxColour(30, 30, 30)));
    dc.Clear();

    int startX = 4;
    int startY = 44;
    int iconSize = 128;

    for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
        int col = i % m_columns;
        int row = i / m_columns;

        int x = startX + col * m_cellSize;
        int y = startY + row * m_rowHeight;

        auto& item = m_items[i];
        bool isDir = std::filesystem::is_directory(item);

        wxBitmap thumb;
        if (isDir) {
            thumb = m_thumbnailCache.GetThumbnail(item, iconSize);
        } else {
            thumb = m_thumbnailCache.GetThumbnail(item, iconSize);
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
        dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                          wxFONTWEIGHT_NORMAL));

        wxCoord textW = 0, textH = 0;
        dc.GetTextExtent(name, &textW, &textH);

        int textX = x + (m_cellSize - textW) / 2;
        int textY = y + iconSize + 4;

        dc.DrawText(name, textX, textY);
    }
}

std::filesystem::path FileBrowserPanel::GetItemAtIndex(int index) const {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index];
    }
    return "";
}

int FileBrowserPanel::GetIndexAtPosition(wxPoint pos) const {
    wxPoint viewStart = GetViewStart();
    int scrollX = 0, scrollY = 0;
    GetScrollPixelsPerUnit(&scrollX, &scrollY);

    int adjX = pos.x + viewStart.x * scrollX;
    int adjY = pos.y + viewStart.y * scrollY;

    int startX = 4;
    int startY = 44;

    if (adjY < startY) return -1;

    int col = (adjX - startX) / m_cellSize;
    int row = (adjY - startY) / m_rowHeight;

    if (col < 0 || col >= m_columns || row < 0) return -1;

    int index = row * m_columns + col;
    if (index >= static_cast<int>(m_items.size())) return -1;

    return index;
}

wxBitmap FileBrowserPanel::GetFileIcon(const std::filesystem::path& path) const {
    return const_cast<ThumbnailCache&>(m_thumbnailCache).GetThumbnail(path, 128);
}

std::string FileBrowserPanel::GetFileDisplayName(const std::filesystem::path& path) const {
    return path.filename().string();
}

bool FileBrowserPanel::MatchesFilter(const std::filesystem::path& path) const {
    if (m_filterIndex == 0) return true;

    auto mediaType = Core::MediaService::DetectMediaType(path);

    switch (m_filterIndex) {
    case 1: return mediaType == Core::MediaType::Image;
    case 2: return mediaType == Core::MediaType::Video;
    case 3: return mediaType == Core::MediaType::Model;
    default: return true;
    }
}

} // namespace Ui

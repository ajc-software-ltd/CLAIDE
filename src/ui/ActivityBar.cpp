// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        ActivityBar.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/ActivityBar.hpp"

#include <wx/dcclient.h>
#include <wx/image.h>

#include <array>
#include <filesystem>
#include <set>

#include <spdlog/spdlog.h>

#include "platform/PlatformPaths.hpp"

namespace Ui {

ActivityBar::ActivityBar(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(48, -1)),
      m_activeMode(ActivityMode::Notepad) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    auto projectRoot = Platform::GetProjectRoot();
    auto iconsDir = projectRoot / "assets" / "icons";

    struct IconDef {
        const char* filename;
        ActivityMode mode;
    };

    IconDef defs[] = {
        {"notepad_icon.png",        ActivityMode::Notepad},
        {"images_icon.png",         ActivityMode::Images},
        {"video_icon.png",          ActivityMode::Video},
        {"3dmodels_icon.png",       ActivityMode::Models},
        {"app_icon.png",            ActivityMode::AI},
        {"settings_icon.png",       ActivityMode::Settings},
    };

    constexpr std::array<ActivityMode, 6> kExpectedModes = {
        ActivityMode::Notepad,
        ActivityMode::Images,
        ActivityMode::Video,
        ActivityMode::Models,
        ActivityMode::AI,
        ActivityMode::Settings
    };
    std::set<ActivityMode> configuredModes;

    for (auto& def : defs) {
        auto iconPath = iconsDir / def.filename;
        IconEntry entry;
        entry.mode = def.mode;
        configuredModes.insert(def.mode);

        if (std::filesystem::exists(iconPath)) {
            wxImage img(iconPath.string(), wxBITMAP_TYPE_PNG);
            if (img.IsOk()) {
                img.Rescale(32, 32, wxIMAGE_QUALITY_HIGH);
                entry.bitmap = wxBitmap(img);

                wxImage inactiveImg = img;
                inactiveImg = inactiveImg.ConvertToGreyscale();
                entry.inactiveBitmap = wxBitmap(inactiveImg);

                entry.activeBitmap = wxBitmap(img);
            }
        }

        if (!entry.bitmap.IsOk()) {
            wxImage fallback(32, 32);
            fallback.SetRGB(wxRect(0, 0, 32, 32), 100, 100, 100);
            entry.bitmap = wxBitmap(fallback);
            wxImage grey = fallback;
            grey = grey.ConvertToGreyscale();
            entry.inactiveBitmap = wxBitmap(grey);
            entry.activeBitmap = wxBitmap(fallback);
        }

        m_icons.push_back(std::move(entry));
    }

    for (auto mode : kExpectedModes) {
        if (!configuredModes.contains(mode)) {
            spdlog::warn("ActivityBar: missing icon mapping for mode {}", static_cast<int>(mode));
        }
    }

    Bind(wxEVT_LEFT_DOWN, &ActivityBar::OnMouse, this);
    Bind(wxEVT_PAINT, &ActivityBar::OnPaint, this);
}

void ActivityBar::SetActiveMode(ActivityMode mode) {
    m_activeMode = mode;
    Refresh();
}

void ActivityBar::OnMouse(wxMouseEvent& event) {
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

void ActivityBar::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxSize sz = GetSize();

    dc.SetBrush(wxBrush(wxColour(30, 30, 30)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, sz.x, sz.y);

    int iconSize = 32;
    int spacing = 8;
    int y = 12;

    for (auto& icon : m_icons) {
        int x = (sz.x - iconSize) / 2;
        icon.hitRect = wxRect(x - 4, y - 4, iconSize + 8, iconSize + 8);

        if (icon.mode == m_activeMode) {
            // Active indicator line
            dc.SetBrush(wxBrush(wxColour(0, 120, 215)));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(0, y - 2, 3, iconSize + 4);

            dc.DrawBitmap(icon.activeBitmap, x, y, true);
        } else {
            dc.DrawBitmap(icon.inactiveBitmap, x, y, true);
        }

        y += iconSize + spacing;
    }

    event.Skip();
}

} // namespace Ui

#include "ui/CanvasPanel.hpp"

#include <utility>

#include <wx/dcbuffer.h>

#include "ui/Theme.hpp"

namespace Ui {

CanvasPanel::CanvasPanel(wxWindow* parent) : wxPanel(parent), m_statusText("Canvas ready") {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(Theme::GetDarkTheme().background);
    Bind(wxEVT_PAINT, &CanvasPanel::OnPaint, this);
}

void CanvasPanel::SetStatusText(wxString text) {
    m_statusText = std::move(text);
    Refresh();
}

void CanvasPanel::SetDocument(Render::CanvasDocument document) {
    m_canvas.Document() = std::move(document);
    Refresh();
}

const Render::Canvas& CanvasPanel::GetCanvas() const {
    return m_canvas;
}

void CanvasPanel::OnPaint([[maybe_unused]] wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(Theme::GetDarkTheme().background));
    dc.Clear();

    dc.SetTextForeground(Theme::GetDarkTheme().text);
    dc.DrawText("Vulkan Canvas (Milestone 2 foundation)", wxPoint(12, 12));
    dc.DrawText(m_statusText, wxPoint(12, 36));
}

} // namespace Ui

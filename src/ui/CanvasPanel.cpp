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

    constexpr int kCellSize = 18;
    const wxColour cellLight(74, 74, 74);
    const wxColour cellDark(58, 58, 58);
    const auto size = GetClientSize();
    for (int y = 0; y < size.GetHeight(); y += kCellSize) {
        for (int x = 0; x < size.GetWidth(); x += kCellSize) {
            const bool lightCell = ((x / kCellSize) + (y / kCellSize)) % 2 == 0;
            dc.SetBrush(wxBrush(lightCell ? cellLight : cellDark));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(x, y, kCellSize, kCellSize);
        }
    }

    dc.SetTextForeground(Theme::GetDarkTheme().text);
    dc.DrawText("Vulkan Canvas (Milestone 2 foundation)", wxPoint(12, 12));
    dc.DrawText(wxString::Format("Runtime: %s", m_canvas.GetRuntimeStatus()), wxPoint(12, 36));
    dc.DrawText(m_statusText, wxPoint(12, 58));
}

} // namespace Ui

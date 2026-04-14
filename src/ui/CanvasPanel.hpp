#pragma once

#include <wx/panel.h>

#include "render/Canvas.hpp"

namespace Ui {

class CanvasPanel : public wxPanel {
public:
    explicit CanvasPanel(wxWindow* parent);

    void SetStatusText(wxString text);
    void SetDocument(Render::CanvasDocument document);
    const Render::Canvas& GetCanvas() const;

private:
    void OnPaint(wxPaintEvent& event);

    Render::Canvas m_canvas;
    wxString m_statusText;
};

} // namespace Ui

#include "render/Canvas.hpp"

namespace Render {

CanvasDocument& Canvas::Document() {
    return m_document;
}

const CanvasDocument& Canvas::Document() const {
    return m_document;
}

CanvasView& Canvas::View() {
    return m_view;
}

const CanvasView& Canvas::View() const {
    return m_view;
}

void Canvas::SetRuntimeReady(bool ready, std::string status) {
    m_runtimeReady = ready;
    m_runtimeStatus = std::move(status);
}

bool Canvas::IsRuntimeReady() const {
    return m_runtimeReady;
}

const std::string& Canvas::GetRuntimeStatus() const {
    return m_runtimeStatus;
}

} // namespace Render

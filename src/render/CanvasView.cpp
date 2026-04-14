#include "render/CanvasView.hpp"

#include <algorithm>

namespace Render {

void CanvasView::SetZoom(float zoom) {
    m_zoom = std::max(0.1F, zoom);
}

float CanvasView::GetZoom() const {
    return m_zoom;
}

void CanvasView::SetPan(float x, float y) {
    m_panX = x;
    m_panY = y;
}

float CanvasView::GetPanX() const {
    return m_panX;
}

float CanvasView::GetPanY() const {
    return m_panY;
}

} // namespace Render

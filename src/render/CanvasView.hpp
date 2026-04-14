#pragma once

namespace Render {

class CanvasView {
public:
    void SetZoom(float zoom);
    float GetZoom() const;

    void SetPan(float x, float y);
    float GetPanX() const;
    float GetPanY() const;

private:
    float m_zoom{1.0F};
    float m_panX{0.0F};
    float m_panY{0.0F};
};

} // namespace Render

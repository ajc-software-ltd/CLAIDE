#include "vulkan/runtime/VulkanText.hpp"

#include <utility>

namespace Render {

std::expected<void, std::string> VulkanText::BuildGlyphAtlas(std::string fontFace, float pointSize) {
    if (fontFace.empty()) {
        return std::unexpected("Glyph atlas requires a font face");
    }
    if (pointSize <= 0.0F) {
        return std::unexpected("Glyph atlas point size must be positive");
    }

    m_fontFace = std::move(fontFace);
    m_pointSize = pointSize;
    m_hasAtlas = true;
    return {};
}

bool VulkanText::HasAtlas() const {
    return m_hasAtlas;
}

const std::string& VulkanText::GetFontFace() const {
    return m_fontFace;
}

float VulkanText::GetPointSize() const {
    return m_pointSize;
}

} // namespace Render

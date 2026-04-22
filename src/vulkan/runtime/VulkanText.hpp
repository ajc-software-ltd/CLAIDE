#pragma once

#include <expected>
#include <string>

namespace Render {

class VulkanText
{
  public:
    std::expected<void, std::string> BuildGlyphAtlas(std::string fontFace, float pointSize);
    bool HasAtlas() const;
    const std::string& GetFontFace() const;
    float GetPointSize() const;

  private:
    bool m_hasAtlas{false};
    std::string m_fontFace;
    float m_pointSize{0.0F};
};

} // namespace Render

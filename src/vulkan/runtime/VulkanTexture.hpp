#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>

namespace Render {

class VulkanTexture
{
  public:
    std::expected<void, std::string> UploadRgba8(const std::uint8_t* pixels, std::size_t bytes, uint32_t width,
                                                 uint32_t height);
    bool IsReady() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

  private:
    bool m_ready{false};
    uint32_t m_width{0};
    uint32_t m_height{0};
};

} // namespace Render

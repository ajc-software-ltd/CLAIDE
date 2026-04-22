#include "vulkan/runtime/VulkanTexture.hpp"

namespace Render {

std::expected<void, std::string> VulkanTexture::UploadRgba8(const std::uint8_t* pixels, std::size_t bytes, uint32_t width,
                                                            uint32_t height) {
    if (pixels == nullptr) {
        return std::unexpected("Texture upload requires non-null pixel buffer");
    }
    if (width == 0 || height == 0) {
        return std::unexpected("Texture dimensions must be non-zero");
    }

    const std::size_t expectedBytes = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U;
    if (bytes < expectedBytes) {
        return std::unexpected("Texture upload buffer is smaller than expected RGBA8 size");
    }

    m_width = width;
    m_height = height;
    m_ready = true;
    return {};
}

bool VulkanTexture::IsReady() const {
    return m_ready;
}

uint32_t VulkanTexture::GetWidth() const {
    return m_width;
}

uint32_t VulkanTexture::GetHeight() const {
    return m_height;
}

} // namespace Render

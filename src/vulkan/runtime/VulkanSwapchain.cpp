#include "vulkan/runtime/VulkanSwapchain.hpp"

namespace Render {

std::expected<void, std::string> VulkanSwapchain::Create(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return std::unexpected("Swapchain dimensions must be non-zero");
    }
    m_width = width;
    m_height = height;
    m_created = true;
    return {};
}

void VulkanSwapchain::Destroy() {
    m_created = false;
    m_width = 0;
    m_height = 0;
}

bool VulkanSwapchain::IsCreated() const {
    return m_created;
}

} // namespace Render

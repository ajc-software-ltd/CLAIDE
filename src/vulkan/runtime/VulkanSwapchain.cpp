#include "vulkan/runtime/VulkanSwapchain.hpp"

namespace Render {

std::expected<void, std::string> VulkanSwapchain::Create(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return std::unexpected("Swapchain dimensions must be non-zero");
    }
    if (m_created) {
        return std::unexpected("Swapchain is already created");
    }
    m_width = width;
    m_height = height;
    m_created = true;
    return {};
}

std::expected<void, std::string> VulkanSwapchain::Recreate(uint32_t width, uint32_t height) {
    if (!m_created) {
        return Create(width, height);
    }
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

uint32_t VulkanSwapchain::GetWidth() const {
    return m_width;
}

uint32_t VulkanSwapchain::GetHeight() const {
    return m_height;
}

} // namespace Render

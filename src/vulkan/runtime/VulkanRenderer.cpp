#include "vulkan/runtime/VulkanRenderer.hpp"

namespace Render {

std::expected<void, std::string> VulkanRenderer::Initialize(VulkanSwapchain* swapchain) {
    if (swapchain == nullptr || !swapchain->IsCreated()) {
        return std::unexpected("Renderer requires a created swapchain");
    }
    m_swapchain = swapchain;
    return {};
}

std::expected<void, std::string> VulkanRenderer::RenderFrame() {
    if (m_swapchain == nullptr || !m_swapchain->IsCreated()) {
        return std::unexpected("Renderer is not initialized");
    }
    return {};
}

void VulkanRenderer::Shutdown() {
    m_swapchain = nullptr;
}

} // namespace Render

#include "vulkan/runtime/VulkanRenderer.hpp"

namespace Render {

std::expected<void, std::string> VulkanRenderer::Initialize(VulkanSwapchain* swapchain) {
    if (swapchain == nullptr || !swapchain->IsCreated()) {
        return std::unexpected("Renderer requires a created swapchain");
    }
    m_swapchain = swapchain;
    m_initialized = true;
    m_frameActive = false;
    return {};
}

std::expected<void, std::string> VulkanRenderer::BeginFrame() {
    if (!m_initialized || m_swapchain == nullptr || !m_swapchain->IsCreated()) {
        return std::unexpected("Renderer is not initialized");
    }
    if (m_frameActive) {
        return std::unexpected("Frame is already active");
    }
    m_frameActive = true;
    return {};
}

std::expected<void, std::string> VulkanRenderer::EndFrame() {
    if (!m_initialized || m_swapchain == nullptr || !m_swapchain->IsCreated()) {
        return std::unexpected("Renderer is not initialized");
    }
    if (!m_frameActive) {
        return std::unexpected("No active frame to end");
    }
    m_frameActive = false;
    return {};
}

std::expected<void, std::string> VulkanRenderer::Present() {
    if (!m_initialized || m_swapchain == nullptr || !m_swapchain->IsCreated()) {
        return std::unexpected("Renderer is not initialized");
    }
    if (m_frameActive) {
        return std::unexpected("Cannot present while frame is active");
    }
    return {};
}

std::expected<void, std::string> VulkanRenderer::RenderFrame() {
    auto begin = BeginFrame();
    if (!begin) {
        return begin;
    }

    auto end = EndFrame();
    if (!end) {
        return end;
    }

    auto present = Present();
    if (!present) {
        return present;
    }
    return {};
}

void VulkanRenderer::Shutdown() {
    m_swapchain = nullptr;
    m_initialized = false;
    m_frameActive = false;
}

bool VulkanRenderer::IsInitialized() const {
    return m_initialized;
}

bool VulkanRenderer::IsFrameActive() const {
    return m_frameActive;
}

} // namespace Render

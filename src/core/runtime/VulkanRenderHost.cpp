#include "core/runtime/VulkanRenderHost.hpp"

namespace Core {

VulkanRenderHost::VulkanRenderHost(VulkanRuntimeLoader* loader) : m_loader(loader), m_surface(nullptr) {
}

std::expected<void, std::string> VulkanRenderHost::Attach(const RenderHostConfig& config) {
    if (m_loader == nullptr || !m_loader->IsLoaded() || !m_loader->IsApiCompatible()) {
        return std::unexpected("VulkanRenderHost: runtime loader is unavailable");
    }
    if (!m_loader->IsVulkanAvailable()) {
        return std::unexpected("VulkanRenderHost: Vulkan is unavailable");
    }

    if (config.nativeWindowHandle == 0) {
        return std::unexpected("VulkanRenderHost: native window handle is invalid");
    }

    m_config = config;
    VulkanAISurfaceDesc desc{
        .width = config.width, .height = config.height, .format = 0, .nativeWindowHandle = config.nativeWindowHandle};
    m_surface = m_loader->CreateSurface(desc);
    if (m_surface == nullptr) {
        return std::unexpected("VulkanRenderHost: failed to create runtime surface");
    }
    return {};
}

std::expected<void, std::string> VulkanRenderHost::Resize(uint32_t width, uint32_t height) {
    m_config.width = width;
    m_config.height = height;

    if (m_surface != nullptr && m_loader != nullptr) {
        m_loader->DestroySurface(m_surface);
        VulkanAISurfaceDesc desc{
            .width = width, .height = height, .format = 0, .nativeWindowHandle = m_config.nativeWindowHandle};
        m_surface = m_loader->CreateSurface(desc);
        if (m_surface == nullptr) {
            return std::unexpected("VulkanRenderHost: failed to recreate runtime surface after resize");
        }
    }
    return {};
}

std::expected<void, std::string> VulkanRenderHost::BeginFrame() {
    if (m_loader == nullptr || m_surface == nullptr) {
        return std::unexpected("VulkanRenderHost: cannot begin frame without surface");
    }

    auto result = m_loader->BeginFrame(m_surface);
    if (result != VULKANAI_OK) {
        return std::unexpected("VulkanRenderHost: BeginFrame failed: " + m_loader->GetLastError());
    }
    return {};
}

std::expected<void, std::string> VulkanRenderHost::EndFrame() {
    if (m_loader == nullptr || m_surface == nullptr) {
        return std::unexpected("VulkanRenderHost: cannot end frame without surface");
    }

    auto result = m_loader->EndFrame(m_surface);
    if (result != VULKANAI_OK) {
        return std::unexpected("VulkanRenderHost: EndFrame failed: " + m_loader->GetLastError());
    }
    return {};
}

std::expected<void, std::string> VulkanRenderHost::Present() {
    if (m_loader == nullptr || m_surface == nullptr) {
        return std::unexpected("VulkanRenderHost: cannot present frame without surface");
    }

    auto result = m_loader->PresentFrame(m_surface);
    if (result != VULKANAI_OK) {
        return std::unexpected("VulkanRenderHost: Present failed: " + m_loader->GetLastError());
    }
    return {};
}

void VulkanRenderHost::Detach() {
    if (m_loader != nullptr && m_surface != nullptr) {
        m_loader->DestroySurface(m_surface);
    }
    m_surface = nullptr;
}

} // namespace Core

#pragma once

#include <expected>
#include <string>

#include "vulkan/runtime/VulkanSwapchain.hpp"

namespace Render {

class VulkanRenderer
{
  public:
    std::expected<void, std::string> Initialize(VulkanSwapchain* swapchain);
    std::expected<void, std::string> BeginFrame();
    std::expected<void, std::string> EndFrame();
    std::expected<void, std::string> Present();
    std::expected<void, std::string> RenderFrame();
    void Shutdown();
    bool IsInitialized() const;
    bool IsFrameActive() const;

  private:
    VulkanSwapchain* m_swapchain{nullptr};
    bool m_initialized{false};
    bool m_frameActive{false};
};

} // namespace Render

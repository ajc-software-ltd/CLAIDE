#pragma once

#include <expected>
#include <string>

#include "vulkan/runtime/VulkanSwapchain.hpp"

namespace Render {

class VulkanRenderer
{
  public:
    std::expected<void, std::string> Initialize(VulkanSwapchain* swapchain);
    std::expected<void, std::string> RenderFrame();
    void Shutdown();

  private:
    VulkanSwapchain* m_swapchain{nullptr};
};

} // namespace Render

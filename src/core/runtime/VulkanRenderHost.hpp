#pragma once

#include <expected>
#include <string>

#include "core/VulkanRuntimeLoader.hpp"
#include "core/runtime/RenderHost.hpp"

namespace Core {

class VulkanRenderHost final : public RenderHost
{
  public:
    explicit VulkanRenderHost(VulkanRuntimeLoader* loader);

    std::expected<void, std::string> Attach(const RenderHostConfig& config) override;
    std::expected<void, std::string> Resize(uint32_t width, uint32_t height) override;
    std::expected<void, std::string> BeginFrame() override;
    std::expected<void, std::string> EndFrame() override;
    std::expected<void, std::string> Present() override;
    void Detach() override;

  private:
    VulkanRuntimeLoader* m_loader;
    VulkanAISurfaceHandle m_surface;
    RenderHostConfig m_config;
};

} // namespace Core

#pragma once

#include <cstdint>
#include <expected>
#include <string>

namespace Render {

class VulkanSwapchain
{
  public:
    std::expected<void, std::string> Create(uint32_t width, uint32_t height);
    void Destroy();
    bool IsCreated() const;

  private:
    bool m_created{false};
    uint32_t m_width{0};
    uint32_t m_height{0};
};

} // namespace Render

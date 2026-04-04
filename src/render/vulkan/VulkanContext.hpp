// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        VulkanContext.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace Render {

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    std::expected<void, std::string> Initialize(bool enableValidation = true);
    [[nodiscard]] bool IsInitialized() const;
    [[nodiscard]] std::string GetDeviceName() const;

    [[nodiscard]] VkInstance GetInstance() const { return m_instance; }
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    [[nodiscard]] VkDevice GetDevice() const { return m_device; }
    [[nodiscard]] VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
    [[nodiscard]] uint32_t GetGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
    [[nodiscard]] VkCommandPool GetCommandPool() const { return m_commandPool; }

private:
    std::expected<void, std::string> CreateInstance(bool enableValidation);
    std::expected<void, std::string> SelectPhysicalDevice();
    std::expected<void, std::string> CreateDevice();
    std::expected<void, std::string> CreateCommandPool();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = 0;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::string m_deviceName;
    bool m_initialized = false;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Render

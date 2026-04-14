// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        VulkanContext.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "render/vulkan/VulkanContext.hpp"

#include <algorithm>
#include <set>

#include <spdlog/spdlog.h>

namespace Render {

#ifndef CLIADE_VERSION_MAJOR
#define CLIADE_VERSION_MAJOR 0
#endif

#ifndef CLIADE_VERSION_MINOR
#define CLIADE_VERSION_MINOR 0
#endif

#ifndef CLIADE_VERSION_PATCH
#define CLIADE_VERSION_PATCH 47
#endif

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData) {
    (void)messageType;
    (void)userData;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::error("VulkanContext: {}", callbackData->pMessage);
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("VulkanContext: {}", callbackData->pMessage);
    } else {
        spdlog::debug("VulkanContext: {}", callbackData->pMessage);
    }

    return VK_FALSE;
}

std::expected<VkDebugUtilsMessengerEXT, std::string> CreateDebugMessenger(
    VkInstance instance, bool enableValidation) {
    if (!enableValidation) {
        return std::expected<VkDebugUtilsMessengerEXT, std::string>(VK_NULL_HANDLE);
    }

    auto createDebugMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (!createDebugMessenger) {
        return std::unexpected("Failed to get vkCreateDebugUtilsMessengerEXT");
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    if (createDebugMessenger(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS) {
        return std::unexpected("Failed to create debug messenger");
    }

    return messenger;
}

} // namespace

struct VulkanContext::Impl {
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
};

VulkanContext::VulkanContext() : m_impl(std::make_unique<Impl>()) {}
VulkanContext::~VulkanContext() {
    if (m_impl->debugMessenger != VK_NULL_HANDLE) {
        auto destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (destroyDebugMessenger) {
            destroyDebugMessenger(m_instance, m_impl->debugMessenger, nullptr);
        }
    }
    if (m_commandPool) vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    if (m_device) vkDestroyDevice(m_device, nullptr);
    if (m_instance) vkDestroyInstance(m_instance, nullptr);
}

std::expected<void, std::string> VulkanContext::Initialize(bool enableValidation) {
    if (m_initialized) return {};

    auto result = CreateInstance(enableValidation);
    if (!result) return result;

    m_impl->debugMessenger = *CreateDebugMessenger(m_instance, enableValidation);

    result = SelectPhysicalDevice();
    if (!result) return result;

    result = CreateDevice();
    if (!result) return result;

    result = CreateCommandPool();
    if (!result) return result;

    m_initialized = true;
    spdlog::info("VulkanContext: initialized, device: {}", m_deviceName);
    return {};
}

bool VulkanContext::IsInitialized() const {
    return m_initialized;
}

std::string VulkanContext::GetDeviceName() const {
    return m_deviceName.empty() ? "None" : m_deviceName;
}

std::expected<void, std::string> VulkanContext::CreateInstance(bool enableValidation) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CLIADE";
    appInfo.applicationVersion = VK_MAKE_VERSION(
        CLIADE_VERSION_MAJOR, CLIADE_VERSION_MINOR, CLIADE_VERSION_PATCH);
    appInfo.pEngineName = "CLIADE Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(
        CLIADE_VERSION_MAJOR, CLIADE_VERSION_MINOR, CLIADE_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions;
    if (enableValidation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidation) {
        const char* validationLayer = "VK_LAYER_KHRONOS_validation";
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &validationLayer;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        return std::unexpected("VulkanContext: failed to create instance");
    }

    spdlog::info("VulkanContext: created instance");
    return {};
}

std::expected<void, std::string> VulkanContext::SelectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        return std::unexpected("VulkanContext: no Vulkan devices found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    int bestScore = -1;
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        int score = 0;
        switch (props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: score = 100; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score = 50; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: score = 25; break;
        default: score = 0; break;
        }

        if (score > bestScore) {
            bestScore = score;
            m_physicalDevice = device;
            m_deviceName = props.deviceName;
        }
    }

    if (!m_physicalDevice) {
        return std::unexpected("VulkanContext: no suitable device found");
    }

    spdlog::info("VulkanContext: selected device: {} (score: {})", m_deviceName, bestScore);
    return {};
}

std::expected<void, std::string> VulkanContext::CreateDevice() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    bool foundGraphics = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphicsQueueFamily = i;
            foundGraphics = true;
            break;
        }
    }

    if (!foundGraphics) {
        return std::unexpected("VulkanContext: no graphics queue family found");
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueCreateInfo;

    if (vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device) != VK_SUCCESS) {
        return std::unexpected("VulkanContext: failed to create logical device");
    }

    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
    spdlog::info("VulkanContext: created logical device");
    return {};
}

std::expected<void, std::string> VulkanContext::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamily;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        return std::unexpected("VulkanContext: failed to create command pool");
    }

    spdlog::info("VulkanContext: created command pool");
    return {};
}

} // namespace Render

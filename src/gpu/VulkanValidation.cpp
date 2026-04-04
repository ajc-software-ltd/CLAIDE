// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        VulkanValidation.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "gpu/VulkanValidation.hpp"

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

namespace Gpu {

std::vector<const char*> VulkanValidation::GetValidationLayers() {
    return {"VK_LAYER_KHRONOS_validation"};
}

std::vector<const char*> VulkanValidation::GetRequiredExtensions(bool enableValidation) {
    std::vector<const char*> extensions;

    if (enableValidation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanValidation::CheckValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : GetValidationLayers()) {
        bool found = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            spdlog::warn("VulkanValidation: validation layer {} not found", layerName);
            return false;
        }
    }

    spdlog::info("VulkanValidation: all validation layers available");
    return true;
}

} // namespace Gpu

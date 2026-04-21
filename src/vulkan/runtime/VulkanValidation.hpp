// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        VulkanValidation.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <string>
#include <vector>

namespace Gpu {

class VulkanValidation
{
  public:
    static std::vector<const char*> GetValidationLayers();
    static std::vector<const char*> GetRequiredExtensions(bool enableValidation);

    static bool CheckValidationLayerSupport();
};

} // namespace Gpu

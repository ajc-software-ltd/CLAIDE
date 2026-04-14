#include "vulkanai/VulkanAI.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

#include <spdlog/spdlog.h>

#ifdef VULKANAI_HAS_VULKAN
#include "vulkan/runtime/VulkanContext.hpp"
#endif

namespace {

constexpr VulkanAIApiVersion k_ApiVersion{
    VULKANAI_API_VERSION_MAJOR,
    VULKANAI_API_VERSION_MINOR};

std::mutex g_mutex;
bool g_initialized = false;
std::string g_lastError;
std::string g_availabilityReason;
VulkanAIAvailabilityReasonCode g_reasonCode = VULKANAI_REASON_NONE;
std::unordered_set<void*> g_surfaces;
#ifdef VULKANAI_HAS_VULKAN
std::unique_ptr<Render::VulkanContext> g_context;
#endif

#ifndef CLIADE_VERSION_MAJOR
#define CLIADE_VERSION_MAJOR 0
#endif

#ifndef CLIADE_VERSION_MINOR
#define CLIADE_VERSION_MINOR 0
#endif

#ifndef CLIADE_VERSION_PATCH
#define CLIADE_VERSION_PATCH 47
#endif

void SetError(std::string message) {
    g_lastError = std::move(message);
    spdlog::warn("VulkanAI: {}", g_lastError);
}

struct SurfaceState {
    uint32_t width = 0;
    uint32_t height = 0;
    uintptr_t nativeWindowHandle = 0;
    bool inFrame = false;
};

} // namespace

VulkanAIResult VulkanAI_Initialize(void) {
    std::lock_guard lock(g_mutex);
    if (g_initialized) {
        SetError("VulkanAI runtime is already initialized");
        return VULKANAI_ERROR_ALREADY_INITIALIZED;
    }

    g_lastError.clear();
#ifdef VULKANAI_HAS_VULKAN
    g_context = std::make_unique<Render::VulkanContext>();
    auto contextResult = g_context->Initialize(true);
    if (!contextResult) {
        g_initialized = false;
        g_reasonCode = VULKANAI_REASON_UNKNOWN;
        g_availabilityReason = "Failed to initialize Vulkan runtime context";
        SetError("Vulkan context init failed: " + contextResult.error());
        g_context.reset();
        return VULKANAI_ERROR_VULKAN_UNAVAILABLE;
    }

    g_initialized = true;
    g_availabilityReason.clear();
    g_reasonCode = VULKANAI_REASON_NONE;
    spdlog::info("VulkanAI runtime initialized");
    return VULKANAI_OK;
#else
    g_initialized = false;
    g_availabilityReason = "Vulkan SDK/runtime was not linked at build time";
    g_reasonCode = VULKANAI_REASON_RUNTIME_NOT_LINKED;
    SetError(g_availabilityReason);
    return VULKANAI_ERROR_VULKAN_UNAVAILABLE;
#endif
}

void VulkanAI_Shutdown(void) {
    std::lock_guard lock(g_mutex);
    for (auto* surface : g_surfaces) {
        delete static_cast<SurfaceState*>(surface);
    }
    g_surfaces.clear();
#ifdef VULKANAI_HAS_VULKAN
    g_context.reset();
#endif
    g_initialized = false;
    spdlog::info("VulkanAI runtime shutdown");
}

VulkanAIApiVersion VulkanAI_GetApiVersion(void) {
    return k_ApiVersion;
}

int VulkanAI_IsApiCompatible(uint32_t requestedMajor, uint32_t requestedMinor) {
    auto runtimeVersion = VulkanAI_GetApiVersion();
    return runtimeVersion.major == requestedMajor &&
           runtimeVersion.minor == requestedMinor;
}

VulkanAIRuntimeCapabilities VulkanAI_GetCapabilities(void) {
    std::lock_guard lock(g_mutex);
    return VulkanAIRuntimeCapabilities{
        .apiVersion = VulkanAI_GetApiVersion(),
        .runtimeVersion = VulkanAI_GetVersion(),
        .isVulkanAvailable = VulkanAI_IsVulkanAvailable(),
        .reasonCode = g_reasonCode};
}

int VulkanAI_IsInitialized(void) {
    std::lock_guard lock(g_mutex);
    return g_initialized ? 1 : 0;
}

int VulkanAI_IsVulkanAvailable(void) {
#ifdef VULKANAI_HAS_VULKAN
    if (g_context) {
        return g_context->IsInitialized() ? 1 : 0;
    }
    return 1;
#else
    return 0;
#endif
}

VulkanAIVersion VulkanAI_GetVersion(void) {
    return VulkanAIVersion{
        static_cast<uint32_t>(CLIADE_VERSION_MAJOR),
        static_cast<uint32_t>(CLIADE_VERSION_MINOR),
        static_cast<uint32_t>(CLIADE_VERSION_PATCH)};
}

const char* VulkanAI_GetLastError(void) {
    std::lock_guard lock(g_mutex);
    return g_lastError.c_str();
}

const char* VulkanAI_GetAvailabilityReason(void) {
    std::lock_guard lock(g_mutex);
    return g_availabilityReason.c_str();
}

VulkanAIAvailabilityReasonCode VulkanAI_GetAvailabilityReasonCode(void) {
    std::lock_guard lock(g_mutex);
    return g_reasonCode;
}

const char* VulkanAI_GetInstallHelpUrl(void) {
    return "https://vulkan.lunarg.com/sdk/home";
}

VulkanAISurfaceHandle VulkanAI_CreateSurfaceWithDesc(const VulkanAISurfaceDesc* desc) {
    if (desc == nullptr) {
        std::lock_guard lock(g_mutex);
        SetError("CreateSurfaceWithDesc failed: descriptor is null");
        return nullptr;
    }

    if (desc->width == 0 || desc->height == 0) {
        std::lock_guard lock(g_mutex);
        SetError("CreateSurfaceWithDesc failed: zero width/height");
        return nullptr;
    }

    if (desc->nativeWindowHandle == 0) {
        std::lock_guard lock(g_mutex);
        SetError("CreateSurfaceWithDesc failed: native window handle is null");
        return nullptr;
    }

    std::lock_guard lock(g_mutex);
    if (!g_initialized) {
        SetError("Cannot create surface before runtime initialization");
        return nullptr;
    }

    auto* surface = new SurfaceState{desc->width, desc->height, desc->nativeWindowHandle, false};
    g_surfaces.insert(surface);
    return surface;
}

VulkanAISurfaceHandle VulkanAI_CreateSurface(uint32_t width, uint32_t height) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) {
        SetError("Cannot create surface before runtime initialization");
        return nullptr;
    }
    if (width == 0 || height == 0) {
        SetError("Cannot create surface with zero width/height");
        return nullptr;
    }

    auto* surface = new SurfaceState{width, height, 0, false};
    g_surfaces.insert(surface);
    return surface;
}

void VulkanAI_DestroySurface(VulkanAISurfaceHandle surface) {
    std::lock_guard lock(g_mutex);
    if (surface == nullptr) return;

    if (!g_surfaces.erase(surface)) return;
    delete static_cast<SurfaceState*>(surface);
}

VulkanAIResult VulkanAI_BeginFrame(VulkanAISurfaceHandle surface) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) {
        SetError("BeginFrame failed: runtime is not initialized");
        return VULKANAI_ERROR_INVALID_STATE;
    }
    if (surface == nullptr || !g_surfaces.count(surface)) {
        SetError("BeginFrame failed: invalid surface");
        return VULKANAI_ERROR_INVALID_ARGUMENT;
    }

    auto* state = static_cast<SurfaceState*>(surface);
    if (state->inFrame) {
        SetError("BeginFrame failed: frame already started");
        return VULKANAI_ERROR_INVALID_STATE;
    }
    state->inFrame = true;
    return VULKANAI_OK;
}

VulkanAIResult VulkanAI_EndFrame(VulkanAISurfaceHandle surface) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) {
        SetError("EndFrame failed: runtime is not initialized");
        return VULKANAI_ERROR_INVALID_STATE;
    }
    if (surface == nullptr || !g_surfaces.count(surface)) {
        SetError("EndFrame failed: invalid surface");
        return VULKANAI_ERROR_INVALID_ARGUMENT;
    }

    auto* state = static_cast<SurfaceState*>(surface);
    if (!state->inFrame) {
        SetError("EndFrame failed: frame was not started");
        return VULKANAI_ERROR_INVALID_STATE;
    }

    state->inFrame = false;
    return VULKANAI_OK;
}

VulkanAIResult VulkanAI_PresentFrame(VulkanAISurfaceHandle surface) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) {
        SetError("PresentFrame failed: runtime is not initialized");
        return VULKANAI_ERROR_INVALID_STATE;
    }
    if (surface == nullptr || !g_surfaces.count(surface)) {
        SetError("PresentFrame failed: invalid surface");
        return VULKANAI_ERROR_INVALID_ARGUMENT;
    }

    auto* state = static_cast<SurfaceState*>(surface);
    if (state->inFrame) {
        SetError("PresentFrame failed: frame is still open");
        return VULKANAI_ERROR_INVALID_STATE;
    }

    return VULKANAI_OK;
}

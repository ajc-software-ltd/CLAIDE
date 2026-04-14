#pragma once

#include <stdint.h>

#if defined(_WIN32)
#if defined(VULKANAI_BUILD)
#define VULKANAI_API __declspec(dllexport)
#else
#define VULKANAI_API __declspec(dllimport)
#endif
#else
#define VULKANAI_API __attribute__((visibility("default")))
#endif

// ABI policy:
// - Existing field order must remain stable.
// - New fields must be appended at the end of structs.
// - Breaking changes require bumping VULKANAI_API_VERSION_MAJOR.
#define VULKANAI_API_VERSION_MAJOR 1u
#define VULKANAI_API_VERSION_MINOR 1u

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VulkanAIResult {
    VULKANAI_OK = 0,
    VULKANAI_ERROR_ALREADY_INITIALIZED = 1,
    VULKANAI_ERROR_VULKAN_UNAVAILABLE = 2,
    VULKANAI_ERROR_INTERNAL = 3,
    VULKANAI_ERROR_INVALID_ARGUMENT = 4,
    VULKANAI_ERROR_INVALID_STATE = 5
} VulkanAIResult;

typedef enum VulkanAIAvailabilityReasonCode {
    VULKANAI_REASON_NONE = 0,
    VULKANAI_REASON_RUNTIME_NOT_LINKED = 1,
    VULKANAI_REASON_API_MISMATCH = 2,
    VULKANAI_REASON_UNKNOWN = 255
} VulkanAIAvailabilityReasonCode;

typedef struct VulkanAIVersion {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} VulkanAIVersion;

typedef struct VulkanAIApiVersion {
    uint32_t major;
    uint32_t minor;
} VulkanAIApiVersion;

typedef struct VulkanAISurfaceDesc {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uintptr_t nativeWindowHandle;
} VulkanAISurfaceDesc;

typedef struct VulkanAIRuntimeCapabilities {
    VulkanAIApiVersion apiVersion;
    VulkanAIVersion runtimeVersion;
    int isVulkanAvailable;
    VulkanAIAvailabilityReasonCode reasonCode;
} VulkanAIRuntimeCapabilities;

typedef void* VulkanAISurfaceHandle;

VULKANAI_API VulkanAIResult VulkanAI_Initialize(void);
VULKANAI_API void VulkanAI_Shutdown(void);
VULKANAI_API VulkanAIApiVersion VulkanAI_GetApiVersion(void);
VULKANAI_API int VulkanAI_IsApiCompatible(uint32_t requestedMajor, uint32_t requestedMinor);
VULKANAI_API VulkanAIRuntimeCapabilities VulkanAI_GetCapabilities(void);
VULKANAI_API int VulkanAI_IsInitialized(void);
VULKANAI_API int VulkanAI_IsVulkanAvailable(void);
VULKANAI_API VulkanAIVersion VulkanAI_GetVersion(void);
VULKANAI_API const char* VulkanAI_GetLastError(void);
VULKANAI_API const char* VulkanAI_GetAvailabilityReason(void);
VULKANAI_API VulkanAIAvailabilityReasonCode VulkanAI_GetAvailabilityReasonCode(void);
VULKANAI_API const char* VulkanAI_GetInstallHelpUrl(void);
VULKANAI_API VulkanAISurfaceHandle VulkanAI_CreateSurfaceWithDesc(const VulkanAISurfaceDesc* desc);
VULKANAI_API VulkanAISurfaceHandle VulkanAI_CreateSurface(uint32_t width, uint32_t height);
VULKANAI_API void VulkanAI_DestroySurface(VulkanAISurfaceHandle surface);
VULKANAI_API VulkanAIResult VulkanAI_BeginFrame(VulkanAISurfaceHandle surface);
VULKANAI_API VulkanAIResult VulkanAI_EndFrame(VulkanAISurfaceHandle surface);
VULKANAI_API VulkanAIResult VulkanAI_PresentFrame(VulkanAISurfaceHandle surface);

#ifdef __cplusplus
}
#endif

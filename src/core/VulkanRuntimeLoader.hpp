#pragma once

#include <expected>
#include <string>
#include <vector>

#include "vulkanai/VulkanAI.h"

namespace Core {

class VulkanRuntimeLoader {
public:
    VulkanRuntimeLoader();
    ~VulkanRuntimeLoader();

    VulkanRuntimeLoader(const VulkanRuntimeLoader&) = delete;
    VulkanRuntimeLoader& operator=(const VulkanRuntimeLoader&) = delete;

    std::expected<void, std::string> Load();
    void Unload();

    bool IsLoaded() const;
    bool IsInitialized() const;
    bool IsApiCompatible() const;
    VulkanAIResult Initialize();
    void Shutdown();
    bool IsVulkanAvailable() const;
    VulkanAIAvailabilityReasonCode GetAvailabilityReasonCode() const;
    VulkanAIRuntimeCapabilities GetCapabilities() const;
    VulkanAISurfaceHandle CreateSurface(const VulkanAISurfaceDesc& desc) const;
    void DestroySurface(VulkanAISurfaceHandle surface) const;
    VulkanAIResult BeginFrame(VulkanAISurfaceHandle surface) const;
    VulkanAIResult EndFrame(VulkanAISurfaceHandle surface) const;
    VulkanAIResult PresentFrame(VulkanAISurfaceHandle surface) const;

    std::string GetLastError() const;
    std::vector<std::string> GetLastLoadAttempts() const;
    std::vector<std::string> GetRuntimeSearchPaths() const;
    std::string GetAvailabilityReason() const;
    std::string GetInstallHelpUrl() const;

private:
    void* m_module;

    using InitializeFn = VulkanAIResult (*)();
    using ShutdownFn = void (*)();
    using GetApiVersionFn = VulkanAIApiVersion (*)();
    using IsApiCompatibleFn = int (*)(uint32_t, uint32_t);
    using IsInitializedFn = int (*)();
    using GetCapabilitiesFn = VulkanAIRuntimeCapabilities (*)();
    using GetVersionFn = VulkanAIVersion (*)();
    using GetLastErrorFn = const char* (*)();
    using IsVulkanAvailableFn = int (*)();
    using GetAvailabilityReasonFn = const char* (*)();
    using GetAvailabilityReasonCodeFn = VulkanAIAvailabilityReasonCode (*)();
    using GetInstallHelpUrlFn = const char* (*)();
    using CreateSurfaceWithDescFn = VulkanAISurfaceHandle (*)(const VulkanAISurfaceDesc*);
    using DestroySurfaceFn = void (*)(VulkanAISurfaceHandle);
    using BeginFrameFn = VulkanAIResult (*)(VulkanAISurfaceHandle);
    using EndFrameFn = VulkanAIResult (*)(VulkanAISurfaceHandle);
    using PresentFrameFn = VulkanAIResult (*)(VulkanAISurfaceHandle);

    InitializeFn m_initialize;
    ShutdownFn m_shutdown;
    GetApiVersionFn m_getApiVersion;
    IsApiCompatibleFn m_isApiCompatible;
    IsInitializedFn m_isInitialized;
    GetCapabilitiesFn m_getCapabilities;
    GetVersionFn m_getVersion;
    GetLastErrorFn m_getLastError;
    IsVulkanAvailableFn m_isVulkanAvailable;
    GetAvailabilityReasonFn m_getAvailabilityReason;
    GetAvailabilityReasonCodeFn m_getAvailabilityReasonCode;
    GetInstallHelpUrlFn m_getInstallHelpUrl;
    CreateSurfaceWithDescFn m_createSurfaceWithDesc;
    DestroySurfaceFn m_destroySurface;
    BeginFrameFn m_beginFrame;
    EndFrameFn m_endFrame;
    PresentFrameFn m_presentFrame;

    std::string m_lastError;
    std::vector<std::string> m_lastLoadAttempts;
    std::vector<std::string> m_runtimeSearchPaths;
    bool m_apiCompatible;

    std::expected<void, std::string> ResolveSymbols();
};

} // namespace Core

#include "core/VulkanRuntimeLoader.hpp"

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <unordered_set>
#include <vector>

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "platform/PlatformPaths.hpp"

namespace Core {

namespace {

constexpr VulkanAIApiVersion k_ExpectedApiVersion{
    VULKANAI_API_VERSION_MAJOR, VULKANAI_API_VERSION_MINOR};

#ifdef _WIN32
constexpr const char* kModuleName = "VulkanAI.dll";
#else
constexpr const char* kModuleName = "libVulkanAI.so";
#endif

void* TryLoadModule(const std::filesystem::path& path) {
#ifdef _WIN32
    return reinterpret_cast<void*>(LoadLibraryW(path.wstring().c_str()));
#else
    // Clear any stale loader error before calling dlopen.
    (void)dlerror();
    return dlopen(path.c_str(), RTLD_NOW);
#endif
}

std::string GetPlatformLoadError() {
#ifdef _WIN32
    auto errorCode = GetLastError();
    if (errorCode == 0) {
        return "unknown error";
    }
    return "Win32 error " + std::to_string(errorCode);
#else
    auto* error = dlerror();
    if (error == nullptr) {
        return "unknown error";
    }
    return error;
#endif
}

} // namespace

VulkanRuntimeLoader::VulkanRuntimeLoader()
    : m_module(nullptr), m_initialize(nullptr), m_shutdown(nullptr),
      m_getApiVersion(nullptr), m_isApiCompatible(nullptr),
      m_isInitialized(nullptr), m_getCapabilities(nullptr), m_getVersion(nullptr),
      m_getLastError(nullptr), m_isVulkanAvailable(nullptr),
      m_getAvailabilityReason(nullptr), m_getAvailabilityReasonCode(nullptr),
      m_getInstallHelpUrl(nullptr),
      m_createSurfaceWithDesc(nullptr), m_destroySurface(nullptr),
      m_beginFrame(nullptr), m_endFrame(nullptr), m_presentFrame(nullptr),
      m_apiCompatible(false) {}

VulkanRuntimeLoader::~VulkanRuntimeLoader() {
    Unload();
}

std::expected<void, std::string> VulkanRuntimeLoader::Load() {
    if (m_module != nullptr) {
        return {};
    }
    m_lastLoadAttempts.clear();
    m_runtimeSearchPaths.clear();

    std::vector<std::filesystem::path> candidates;

    if (const char* overridePath = std::getenv("CLIADE_VULKANAI_PATH");
        overridePath != nullptr && *overridePath != '\0') {
        candidates.emplace_back(overridePath);
    }

    candidates.emplace_back(kModuleName);

    auto projectRoot = Platform::GetProjectRoot();
    candidates.push_back(projectRoot / "bin" / kModuleName);
    candidates.push_back(projectRoot / "build" / kModuleName);
    candidates.push_back(projectRoot / "build" / "Debug" / kModuleName);
    candidates.push_back(projectRoot / "build" / "Release" / kModuleName);
    candidates.push_back(projectRoot / "lib" / kModuleName);

    std::error_code cwdEc;
    auto cwd = std::filesystem::current_path(cwdEc);
    if (!cwdEc) {
        candidates.push_back(cwd / kModuleName);
        candidates.push_back(cwd / "bin" / kModuleName);
        candidates.push_back(cwd / "lib" / kModuleName);
    }

    std::vector<std::filesystem::path> uniqueCandidates;
    std::unordered_set<std::string> seen;
    uniqueCandidates.reserve(candidates.size());
    for (const auto& candidate : candidates) {
        auto key = candidate.lexically_normal().string();
        if (seen.insert(key).second) {
            uniqueCandidates.push_back(candidate);
            m_runtimeSearchPaths.push_back(candidate.string());
        }
    }

    std::vector<std::string> loadErrors;
    for (const auto& candidate : uniqueCandidates) {
        m_module = TryLoadModule(candidate);
        if (m_module != nullptr) {
            spdlog::info("VulkanRuntimeLoader::Load: loaded runtime from {}",
                         candidate.string());
            break;
        }
        auto attempt = candidate.string() + " (" + GetPlatformLoadError() + ")";
        m_lastLoadAttempts.push_back(attempt);
        loadErrors.push_back(std::move(attempt));
    }

    if (m_module == nullptr) {
        std::ostringstream oss;
        oss << "Failed to load Vulkan runtime module. Tried: ";
        for (std::size_t i = 0; i < loadErrors.size(); ++i) {
            if (i > 0) {
                oss << "; ";
            }
            oss << loadErrors[i];
        }
        m_lastError = oss.str();
        spdlog::warn("VulkanRuntimeLoader::Load: {}", m_lastError);
        return std::unexpected(m_lastError);
    }

    auto symbolResult = ResolveSymbols();
    if (!symbolResult) {
        Unload();
        return std::unexpected(symbolResult.error());
    }

    auto apiVersion = m_getApiVersion();
    m_apiCompatible = m_isApiCompatible(
                          k_ExpectedApiVersion.major, k_ExpectedApiVersion.minor) == 1;
    if (!m_apiCompatible) {
        m_lastError = "Vulkan runtime API version mismatch";
        spdlog::error("VulkanRuntimeLoader::Load: expected {}.{}, got {}.{}",
                      k_ExpectedApiVersion.major, k_ExpectedApiVersion.minor,
                      apiVersion.major, apiVersion.minor);
        Unload();
        return std::unexpected(m_lastError);
    }

    return {};
}

void VulkanRuntimeLoader::Unload() {
    if (m_module == nullptr) return;

#ifdef _WIN32
    FreeLibrary(reinterpret_cast<HMODULE>(m_module));
#else
    dlclose(m_module);
#endif

    m_module = nullptr;
    m_initialize = nullptr;
    m_shutdown = nullptr;
    m_getApiVersion = nullptr;
    m_isApiCompatible = nullptr;
    m_isInitialized = nullptr;
    m_getCapabilities = nullptr;
    m_getVersion = nullptr;
    m_getLastError = nullptr;
    m_isVulkanAvailable = nullptr;
    m_getAvailabilityReason = nullptr;
    m_getAvailabilityReasonCode = nullptr;
    m_getInstallHelpUrl = nullptr;
    m_createSurfaceWithDesc = nullptr;
    m_destroySurface = nullptr;
    m_beginFrame = nullptr;
    m_endFrame = nullptr;
    m_presentFrame = nullptr;
    m_apiCompatible = false;
}

bool VulkanRuntimeLoader::IsLoaded() const {
    return m_module != nullptr;
}

bool VulkanRuntimeLoader::IsInitialized() const {
    if (m_isInitialized == nullptr) return false;
    return m_isInitialized() == 1;
}

bool VulkanRuntimeLoader::IsApiCompatible() const {
    return m_apiCompatible;
}

VulkanAIResult VulkanRuntimeLoader::Initialize() {
    if (m_initialize == nullptr) return VULKANAI_ERROR_INTERNAL;
    return m_initialize();
}

void VulkanRuntimeLoader::Shutdown() {
    if (m_shutdown != nullptr) {
        m_shutdown();
    }
}

bool VulkanRuntimeLoader::IsVulkanAvailable() const {
    if (m_isVulkanAvailable == nullptr) return false;
    return m_isVulkanAvailable() == 1;
}

VulkanAIAvailabilityReasonCode VulkanRuntimeLoader::GetAvailabilityReasonCode() const {
    if (m_getAvailabilityReasonCode == nullptr) {
        return VULKANAI_REASON_UNKNOWN;
    }
    return m_getAvailabilityReasonCode();
}

VulkanAIRuntimeCapabilities VulkanRuntimeLoader::GetCapabilities() const {
    if (m_getCapabilities == nullptr) {
        return VulkanAIRuntimeCapabilities{
            .apiVersion = VulkanAIApiVersion{0, 0},
            .runtimeVersion = VulkanAIVersion{0, 0, 0},
            .isVulkanAvailable = 0,
            .reasonCode = VULKANAI_REASON_UNKNOWN};
    }

    return m_getCapabilities();
}

std::string VulkanRuntimeLoader::GetLastError() const {
    if (m_getLastError == nullptr) {
        return m_lastError;
    }

    auto err = m_getLastError();
    if (err == nullptr) {
        return m_lastError;
    }
    return err;
}

std::vector<std::string> VulkanRuntimeLoader::GetLastLoadAttempts() const {
    return m_lastLoadAttempts;
}

std::vector<std::string> VulkanRuntimeLoader::GetRuntimeSearchPaths() const {
    return m_runtimeSearchPaths;
}

std::string VulkanRuntimeLoader::GetAvailabilityReason() const {
    if (m_getAvailabilityReason == nullptr) {
        return "Vulkan availability reason is not available";
    }

    auto reason = m_getAvailabilityReason();
    if (reason == nullptr) {
        return "Vulkan availability reason is not available";
    }
    return reason;
}

std::string VulkanRuntimeLoader::GetInstallHelpUrl() const {
    if (m_getInstallHelpUrl == nullptr) {
        return "https://vulkan.lunarg.com/sdk/home";
    }

    auto url = m_getInstallHelpUrl();
    if (url == nullptr) {
        return "https://vulkan.lunarg.com/sdk/home";
    }
    return url;
}

VulkanAISurfaceHandle VulkanRuntimeLoader::CreateSurface(const VulkanAISurfaceDesc& desc) const {
    if (m_createSurfaceWithDesc == nullptr) return nullptr;
    return m_createSurfaceWithDesc(&desc);
}

void VulkanRuntimeLoader::DestroySurface(VulkanAISurfaceHandle surface) const {
    if (m_destroySurface == nullptr) return;
    m_destroySurface(surface);
}

VulkanAIResult VulkanRuntimeLoader::BeginFrame(VulkanAISurfaceHandle surface) const {
    if (m_beginFrame == nullptr) return VULKANAI_ERROR_INTERNAL;
    return m_beginFrame(surface);
}

VulkanAIResult VulkanRuntimeLoader::EndFrame(VulkanAISurfaceHandle surface) const {
    if (m_endFrame == nullptr) return VULKANAI_ERROR_INTERNAL;
    return m_endFrame(surface);
}

VulkanAIResult VulkanRuntimeLoader::PresentFrame(VulkanAISurfaceHandle surface) const {
    if (m_presentFrame == nullptr) return VULKANAI_ERROR_INTERNAL;
    return m_presentFrame(surface);
}

std::expected<void, std::string> VulkanRuntimeLoader::ResolveSymbols() {
    if (m_module == nullptr) {
        return std::unexpected("Runtime module is not loaded");
    }

    auto loadSymbol = [this](const char* symbolName) -> void* {
#ifdef _WIN32
        return reinterpret_cast<void*>(GetProcAddress(
            reinterpret_cast<HMODULE>(m_module), symbolName));
#else
        return dlsym(m_module, symbolName);
#endif
    };

    m_initialize = reinterpret_cast<InitializeFn>(loadSymbol("VulkanAI_Initialize"));
    m_shutdown = reinterpret_cast<ShutdownFn>(loadSymbol("VulkanAI_Shutdown"));
    m_getApiVersion = reinterpret_cast<GetApiVersionFn>(loadSymbol("VulkanAI_GetApiVersion"));
    m_isApiCompatible =
        reinterpret_cast<IsApiCompatibleFn>(loadSymbol("VulkanAI_IsApiCompatible"));
    m_isInitialized = reinterpret_cast<IsInitializedFn>(loadSymbol("VulkanAI_IsInitialized"));
    m_getCapabilities = reinterpret_cast<GetCapabilitiesFn>(loadSymbol("VulkanAI_GetCapabilities"));
    m_isVulkanAvailable = reinterpret_cast<IsVulkanAvailableFn>(loadSymbol("VulkanAI_IsVulkanAvailable"));
    m_getVersion = reinterpret_cast<GetVersionFn>(loadSymbol("VulkanAI_GetVersion"));
    m_getLastError = reinterpret_cast<GetLastErrorFn>(loadSymbol("VulkanAI_GetLastError"));
    m_getAvailabilityReason = reinterpret_cast<GetAvailabilityReasonFn>(loadSymbol("VulkanAI_GetAvailabilityReason"));
    m_getAvailabilityReasonCode = reinterpret_cast<GetAvailabilityReasonCodeFn>(
        loadSymbol("VulkanAI_GetAvailabilityReasonCode"));
    m_getInstallHelpUrl = reinterpret_cast<GetInstallHelpUrlFn>(loadSymbol("VulkanAI_GetInstallHelpUrl"));
    m_createSurfaceWithDesc = reinterpret_cast<CreateSurfaceWithDescFn>(loadSymbol("VulkanAI_CreateSurfaceWithDesc"));
    m_destroySurface = reinterpret_cast<DestroySurfaceFn>(loadSymbol("VulkanAI_DestroySurface"));
    m_beginFrame = reinterpret_cast<BeginFrameFn>(loadSymbol("VulkanAI_BeginFrame"));
    m_endFrame = reinterpret_cast<EndFrameFn>(loadSymbol("VulkanAI_EndFrame"));
    m_presentFrame = reinterpret_cast<PresentFrameFn>(loadSymbol("VulkanAI_PresentFrame"));

    if (m_initialize == nullptr || m_shutdown == nullptr ||
        m_getApiVersion == nullptr || m_isApiCompatible == nullptr ||
        m_isInitialized == nullptr || m_getCapabilities == nullptr ||
        m_isVulkanAvailable == nullptr ||
        m_getVersion == nullptr || m_getLastError == nullptr ||
        m_getAvailabilityReason == nullptr || m_getAvailabilityReasonCode == nullptr ||
        m_getInstallHelpUrl == nullptr ||
        m_createSurfaceWithDesc == nullptr || m_destroySurface == nullptr ||
        m_beginFrame == nullptr || m_endFrame == nullptr ||
        m_presentFrame == nullptr) {
        m_lastError = "Failed to resolve one or more Vulkan runtime symbols";
        spdlog::error("VulkanRuntimeLoader::ResolveSymbols: {}", m_lastError);
        return std::unexpected(m_lastError);
    }

    return {};
}

} // namespace Core

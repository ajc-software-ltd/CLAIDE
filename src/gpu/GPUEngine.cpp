// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        GPUEngine.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "gpu/GPUEngine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <unordered_map>

#include <spdlog/spdlog.h>

#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif
#include <vulkan/vulkan.h>

namespace Gpu {

#ifndef CLIADE_VERSION_MAJOR
#define CLIADE_VERSION_MAJOR 0
#endif

#ifndef CLIADE_VERSION_MINOR
#define CLIADE_VERSION_MINOR 0
#endif

#ifndef CLIADE_VERSION_PATCH
#define CLIADE_VERSION_PATCH 47
#endif

struct GPUEngine::Impl {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    std::string deviceName;
    bool initialized = false;
    FilterExecutionPath lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
    std::unordered_map<std::string, std::filesystem::path> shaderPaths;
    std::unordered_map<std::string, bool> shaderAvailability;

    ~Impl() {
        if (commandPool) vkDestroyCommandPool(device, commandPool, nullptr);
        if (pipelineLayout) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        if (device) vkDestroyDevice(device, nullptr);
        if (instance) vkDestroyInstance(instance, nullptr);
    }
};

GPUEngine::GPUEngine() : m_impl(std::make_unique<Impl>()) {}
GPUEngine::~GPUEngine() = default;

bool GPUEngine::Initialize() {
    if (m_impl->initialized) return true;

    // Create instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "CLIADE";
    appInfo.applicationVersion = VK_MAKE_VERSION(
        CLIADE_VERSION_MAJOR, CLIADE_VERSION_MINOR, CLIADE_VERSION_PATCH);
    appInfo.pEngineName = "CLIADE GPU Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(
        CLIADE_VERSION_MAJOR, CLIADE_VERSION_MINOR, CLIADE_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (vkCreateInstance(&createInfo, nullptr, &m_impl->instance) != VK_SUCCESS) {
        spdlog::warn("GPUEngine::Initialize: failed to create Vulkan instance, using CPU fallback");
        return false;
    }

    // Enumerate physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        spdlog::warn("GPUEngine::Initialize: no Vulkan devices found, using CPU fallback");
        vkDestroyInstance(m_impl->instance, nullptr);
        m_impl->instance = VK_NULL_HANDLE;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, devices.data());

    // Select best device (prefer discrete GPU)
    int bestScore = -1;
    for (auto device : devices) {
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
            m_impl->physicalDevice = device;
            m_impl->deviceName = props.deviceName;
        }
    }

    if (!m_impl->physicalDevice) {
        spdlog::warn("GPUEngine::Initialize: no suitable device, using CPU fallback");
        vkDestroyInstance(m_impl->instance, nullptr);
        m_impl->instance = VK_NULL_HANDLE;
        return false;
    }

    // Find compute queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_impl->physicalDevice, &queueFamilyCount, queueFamilies.data());

    bool foundCompute = false;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            m_impl->queueFamilyIndex = i;
            foundCompute = true;
            break;
        }
    }

    if (!foundCompute) {
        spdlog::warn("GPUEngine::Initialize: no compute queue, using CPU fallback");
        vkDestroyInstance(m_impl->instance, nullptr);
        m_impl->instance = VK_NULL_HANDLE;
        return false;
    }

    // Create logical device
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_impl->queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    if (vkCreateDevice(m_impl->physicalDevice, &deviceCreateInfo, nullptr, &m_impl->device) != VK_SUCCESS) {
        spdlog::warn("GPUEngine::Initialize: failed to create device, using CPU fallback");
        vkDestroyInstance(m_impl->instance, nullptr);
        m_impl->instance = VK_NULL_HANDLE;
        return false;
    }

    vkGetDeviceQueue(m_impl->device, m_impl->queueFamilyIndex, 0, &m_impl->queue);

    // Create command pool
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_impl->queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    if (vkCreateCommandPool(m_impl->device, &poolInfo, nullptr, &m_impl->commandPool) != VK_SUCCESS) {
        spdlog::error("GPUEngine::Initialize: failed to create command pool");
        return false;
    }

    // Create descriptor set layout
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = 2;
    binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    if (vkCreateDescriptorSetLayout(m_impl->device, &layoutInfo, nullptr, &m_impl->descriptorSetLayout) != VK_SUCCESS) {
        spdlog::error("GPUEngine::Initialize: failed to create descriptor set layout");
        return false;
    }

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_impl->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(m_impl->device, &pipelineLayoutInfo, nullptr, &m_impl->pipelineLayout) != VK_SUCCESS) {
        spdlog::error("GPUEngine::Initialize: failed to create pipeline layout");
        return false;
    }

    m_impl->initialized = true;
    spdlog::info("GPUEngine::Initialize: Vulkan device: {}", m_impl->deviceName);

    constexpr std::array<const char*, 6> kShaderNames = {
        "brightness", "contrast", "grayscale", "invert", "blur", "sharpen"
    };

    auto buildShaderDir = std::filesystem::current_path() / "build" / "shaders";
    auto sourceShaderDir = std::filesystem::current_path() / "src" / "gpu" / "shaders";

    for (const auto* shaderName : kShaderNames) {
        auto compiledPath = buildShaderDir / (std::string(shaderName) + ".spv");
        auto sourcePath = sourceShaderDir / (std::string(shaderName) + ".spv");

        if (std::filesystem::exists(compiledPath)) {
            m_impl->shaderPaths[shaderName] = compiledPath;
            m_impl->shaderAvailability[shaderName] = true;
            spdlog::debug("GPUEngine::Initialize: shader available: {}", compiledPath.string());
            continue;
        }

        if (std::filesystem::exists(sourcePath)) {
            m_impl->shaderPaths[shaderName] = sourcePath;
            m_impl->shaderAvailability[shaderName] = true;
            spdlog::debug("GPUEngine::Initialize: shader available: {}", sourcePath.string());
            continue;
        }

        m_impl->shaderAvailability[shaderName] = false;
        spdlog::warn("GPUEngine::Initialize: missing shader '{}.spv' in build/shaders or src/gpu/shaders", shaderName);
    }

    return true;
}

bool GPUEngine::IsAvailable() const {
    return m_impl->initialized;
}

std::string GPUEngine::GetDeviceInfo() const {
    return m_impl->deviceName.empty() ? "CPU Fallback" : m_impl->deviceName;
}

FilterExecutionPath GPUEngine::GetLastExecutionPath() const {
    return m_impl->lastExecutionPath;
}

// CPU fallback implementations
static std::vector<std::uint8_t> ApplyBrightnessCPU(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double value) {
    std::vector<std::uint8_t> output = input;
    double factor = value / 100.0;

    for (std::size_t i = 0; i < output.size(); i += channels) {
        for (int c = 0; c < channels; ++c) {
            double v = output[i + c] + factor * 255.0;
            output[i + c] = static_cast<std::uint8_t>(std::max(0.0, std::min(255.0, v)));
        }
    }
    return output;
}

static std::vector<std::uint8_t> ApplyContrastCPU(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double value) {
    std::vector<std::uint8_t> output = input;
    double factor = (259.0 * (value + 255.0)) / (255.0 * (259.0 - value));

    for (std::size_t i = 0; i < output.size(); i += channels) {
        for (int c = 0; c < channels; ++c) {
            double v = factor * (output[i + c] - 128.0) + 128.0;
            output[i + c] = static_cast<std::uint8_t>(std::max(0.0, std::min(255.0, v)));
        }
    }
    return output;
}

static std::vector<std::uint8_t> ApplyGrayscaleCPU(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels) {
    std::vector<std::uint8_t> output = input;

    for (std::size_t i = 0; i < output.size(); i += channels) {
        double gray = 0.299 * output[i] + 0.587 * output[i + 1] + 0.114 * output[i + 2];
        output[i] = output[i + 1] = output[i + 2] = static_cast<std::uint8_t>(gray);
    }
    return output;
}

static std::vector<std::uint8_t> ApplyInvertCPU(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels) {
    std::vector<std::uint8_t> output = input;

    for (std::size_t i = 0; i < output.size(); i += channels) {
        for (int c = 0; c < std::min(channels, 3); ++c) {
            output[i + c] = 255 - output[i + c];
        }
    }
    return output;
}

static std::vector<std::uint8_t> ApplyBlurCPU(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double radius) {
    int kernelSize = static_cast<int>(std::max(1.0, radius * 2 + 1));
    std::vector<std::uint8_t> output(input.size());

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double sum[4] = {0, 0, 0, 0};
            double weightSum = 0;

            for (int ky = -kernelSize; ky <= kernelSize; ++ky) {
                for (int kx = -kernelSize; kx <= kernelSize; ++kx) {
                    int sx = std::max(0, std::min(width - 1, x + kx));
                    int sy = std::max(0, std::min(height - 1, y + ky));
                    double dist = std::sqrt(kx * kx + ky * ky);
                    double w = std::exp(-(dist * dist) / (2.0 * radius * radius));

                    for (int c = 0; c < channels; ++c) {
                        sum[c] += input[(sy * width + sx) * channels + c] * w;
                    }
                    weightSum += w;
                }
            }

            for (int c = 0; c < channels; ++c) {
                output[(y * width + x) * channels + c] =
                    static_cast<std::uint8_t>(std::round(sum[c] / weightSum));
            }
        }
    }
    return output;
}

static std::vector<std::uint8_t> ApplySharpenCPU(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double amount) {
    std::vector<std::uint8_t> output = input;
    double kernel[9] = {0, -amount, 0, -amount, 1 + 4 * amount, -amount, 0, -amount, 0};

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            for (int c = 0; c < channels; ++c) {
                double sum = 0;
                int ki = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        sum += input[((y + ky) * width + (x + kx)) * channels + c] * kernel[ki++];
                    }
                }
                output[(y * width + x) * channels + c] =
                    static_cast<std::uint8_t>(std::max(0.0, std::min(255.0, sum)));
            }
        }
    }
    return output;
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::ApplyBrightness(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double value) {
    if (!m_impl->initialized) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
        return ApplyBrightnessCPU(input, width, height, channels, value);
    }
    return DispatchShader("brightness", input, width, height, channels, {static_cast<float>(value)});
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::ApplyContrast(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double value) {
    if (!m_impl->initialized) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
        return ApplyContrastCPU(input, width, height, channels, value);
    }
    return DispatchShader("contrast", input, width, height, channels, {static_cast<float>(value)});
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::ApplyGrayscale(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels) {
    if (!m_impl->initialized) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
        return ApplyGrayscaleCPU(input, width, height, channels);
    }
    return DispatchShader("grayscale", input, width, height, channels, {});
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::ApplyInvert(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels) {
    if (!m_impl->initialized) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
        return ApplyInvertCPU(input, width, height, channels);
    }
    return DispatchShader("invert", input, width, height, channels, {});
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::ApplyBlur(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double radius) {
    if (!m_impl->initialized) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
        return ApplyBlurCPU(input, width, height, channels, radius);
    }
    return DispatchShader("blur", input, width, height, channels, {static_cast<float>(radius)});
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::ApplySharpen(
    const std::vector<std::uint8_t>& input, [[maybe_unused]] int width, [[maybe_unused]] int height, int channels, double amount) {
    if (!m_impl->initialized) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuUnavailable;
        return ApplySharpenCPU(input, width, height, channels, amount);
    }
    return DispatchShader("sharpen", input, width, height, channels, {static_cast<float>(amount)});
}

std::expected<std::vector<std::uint8_t>, std::string> GPUEngine::DispatchShader(
    const std::string& shaderName,
    const std::vector<std::uint8_t>& input,
    int width, int height, int channels,
    const std::vector<float>& pushConstants) {
    if (!m_impl->initialized) {
        return std::unexpected("GPU not initialized");
    }

    auto it = m_impl->shaderAvailability.find(shaderName);
    if (it == m_impl->shaderAvailability.end() || !it->second) {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackShaderMissing;
        spdlog::warn("GPUEngine::DispatchShader: missing shader '{}.spv', using CPU fallback",
                     shaderName);
    } else {
        m_impl->lastExecutionPath = FilterExecutionPath::CpuFallbackGpuPipelinePending;
        spdlog::info("GPUEngine::DispatchShader: shader '{}' found at {}, GPU dispatch pipeline pending, using CPU fallback",
                     shaderName, m_impl->shaderPaths[shaderName].string());
    }

    if (shaderName == "brightness" && !pushConstants.empty()) {
        return ApplyBrightnessCPU(input, width, height, channels, pushConstants[0]);
    } else if (shaderName == "contrast" && !pushConstants.empty()) {
        return ApplyContrastCPU(input, width, height, channels, pushConstants[0]);
    } else if (shaderName == "grayscale") {
        return ApplyGrayscaleCPU(input, width, height, channels);
    } else if (shaderName == "invert") {
        return ApplyInvertCPU(input, width, height, channels);
    } else if (shaderName == "blur" && !pushConstants.empty()) {
        return ApplyBlurCPU(input, width, height, channels, pushConstants[0]);
    } else if (shaderName == "sharpen" && !pushConstants.empty()) {
        return ApplySharpenCPU(input, width, height, channels, pushConstants[0]);
    }

    return std::unexpected("Unknown shader: " + shaderName);
}

} // namespace Gpu

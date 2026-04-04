// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        GPUEngine.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace Gpu {

class GPUEngine {
public:
    GPUEngine();
    ~GPUEngine();

    bool Initialize();
    [[nodiscard]] bool IsAvailable() const;
    [[nodiscard]] std::string GetDeviceInfo() const;

    std::expected<std::vector<std::uint8_t>, std::string> ApplyBrightness(
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels, double value);

    std::expected<std::vector<std::uint8_t>, std::string> ApplyContrast(
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels, double value);

    std::expected<std::vector<std::uint8_t>, std::string> ApplyGrayscale(
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels);

    std::expected<std::vector<std::uint8_t>, std::string> ApplyInvert(
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels);

    std::expected<std::vector<std::uint8_t>, std::string> ApplyBlur(
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels, double radius);

    std::expected<std::vector<std::uint8_t>, std::string> ApplySharpen(
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels, double amount);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    std::expected<std::vector<std::uint8_t>, std::string> DispatchShader(
        const std::string& shaderName,
        const std::vector<std::uint8_t>& input,
        int width, int height, int channels,
        const std::vector<float>& pushConstants);
};

} // namespace Gpu

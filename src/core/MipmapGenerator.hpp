// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        MipmapGenerator.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <cstdint>
#include <vector>

namespace Core {

class MipmapGenerator {
public:
    static std::vector<std::vector<std::uint8_t>> Generate(
        const std::uint8_t* data, int width, int height, int channels);

    static int GetLevelCount(int width, int height);

    static const std::uint8_t* GetLevel(
        const std::vector<std::vector<std::uint8_t>>& mipmaps,
        int level, int& outWidth, int& outHeight, int channels);

    static std::vector<std::uint8_t> GetLevelCopy(
        const std::vector<std::vector<std::uint8_t>>& mipmaps,
        int level, int& outWidth, int& outHeight, int channels);

private:
    static std::vector<std::uint8_t> Downsample(
        const std::uint8_t* input, int inWidth, int inHeight,
        int outWidth, int outHeight, int channels);
};

} // namespace Core

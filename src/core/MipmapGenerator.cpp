// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        MipmapGenerator.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/MipmapGenerator.hpp"

#include <algorithm>
#include <cmath>

#include <spdlog/spdlog.h>

namespace Core {

std::vector<std::vector<std::uint8_t>> MipmapGenerator::Generate(
    const std::uint8_t* data, int width, int height, int channels) {
    int levels = GetLevelCount(width, height);
    std::vector<std::vector<std::uint8_t>> mipmaps;
    mipmaps.reserve(levels);

    std::size_t levelSize = static_cast<std::size_t>(width) * height * channels;
    mipmaps.emplace_back(data, data + levelSize);

    int curWidth = width;
    int curHeight = height;

    for (int i = 1; i < levels; ++i) {
        int nextWidth = std::max(1, curWidth / 2);
        int nextHeight = std::max(1, curHeight / 2);

        auto level = Downsample(mipmaps.back().data(), curWidth, curHeight,
                                nextWidth, nextHeight, channels);
        mipmaps.push_back(std::move(level));

        curWidth = nextWidth;
        curHeight = nextHeight;
    }

    spdlog::debug("MipmapGenerator::Generate: {} levels, {}x{} -> {}x{}",
                  levels, width, height, curWidth, curHeight);
    return mipmaps;
}

int MipmapGenerator::GetLevelCount(int width, int height) {
    return static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
}

const std::uint8_t* MipmapGenerator::GetLevel(
    const std::vector<std::vector<std::uint8_t>>& mipmaps,
    int level, int& outWidth, int& outHeight, int channels) {
    if (level < 0 || level >= static_cast<int>(mipmaps.size())) {
        return nullptr;
    }

    int baseWidth = 1, baseHeight = 1;
    if (!mipmaps.empty()) {
        std::size_t level0Size = mipmaps[0].size();
        baseWidth = static_cast<int>(std::sqrt(level0Size / channels));
        baseHeight = static_cast<int>(level0Size / (baseWidth * channels));
    }

    outWidth = std::max(1, baseWidth >> level);
    outHeight = std::max(1, baseHeight >> level);

    return mipmaps[level].data();
}

std::vector<std::uint8_t> MipmapGenerator::GetLevelCopy(
    const std::vector<std::vector<std::uint8_t>>& mipmaps,
    int level, int& outWidth, int& outHeight, int channels) {
    if (level < 0 || level >= static_cast<int>(mipmaps.size())) {
        return {};
    }

    int baseWidth = 1, baseHeight = 1;
    if (!mipmaps.empty()) {
        std::size_t level0Size = mipmaps[0].size();
        baseWidth = static_cast<int>(std::sqrt(level0Size / channels));
        baseHeight = static_cast<int>(level0Size / (baseWidth * channels));
    }

    outWidth = std::max(1, baseWidth >> level);
    outHeight = std::max(1, baseHeight >> level);

    return mipmaps[level];
}

std::vector<std::uint8_t> MipmapGenerator::Downsample(
    const std::uint8_t* input, int inWidth, int inHeight,
    int outWidth, int outHeight, int channels) {
    std::vector<std::uint8_t> output(
        static_cast<std::size_t>(outWidth) * outHeight * channels);

    float scaleX = static_cast<float>(inWidth) / outWidth;
    float scaleY = static_cast<float>(inHeight) / outHeight;

    for (int y = 0; y < outHeight; ++y) {
        for (int x = 0; x < outWidth; ++x) {
            float srcX = (x + 0.5f) * scaleX - 0.5f;
            float srcY = (y + 0.5f) * scaleY - 0.5f;

            int x0 = static_cast<int>(std::floor(srcX));
            int y0 = static_cast<int>(std::floor(srcY));
            int x1 = std::min(x0 + 1, inWidth - 1);
            int y1 = std::min(y0 + 1, inHeight - 1);
            x0 = std::max(0, x0);
            y0 = std::max(0, y0);

            float fx = srcX - x0;
            float fy = srcY - y0;

            for (int c = 0; c < channels; ++c) {
                float v00 = input[(y0 * inWidth + x0) * channels + c];
                float v10 = input[(y0 * inWidth + x1) * channels + c];
                float v01 = input[(y1 * inWidth + x0) * channels + c];
                float v11 = input[(y1 * inWidth + x1) * channels + c];

                float top = v00 * (1.0f - fx) + v10 * fx;
                float bottom = v01 * (1.0f - fx) + v11 * fx;
                float value = top * (1.0f - fy) + bottom * fy;

                output[(y * outWidth + x) * channels + c] =
                    static_cast<std::uint8_t>(std::round(value));
            }
        }
    }

    return output;
}

} // namespace Core

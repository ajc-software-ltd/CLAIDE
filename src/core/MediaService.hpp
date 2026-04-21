// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MediaService.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Core {

enum class MediaType { Image, Video, Audio, Model, Text, Unknown };

struct ImageMetadata
{
    int width;
    int height;
    int colorDepth;
    std::string format;
    std::uint64_t fileSize;
};

class MediaService
{
  public:
    static MediaType DetectMediaType(const std::filesystem::path& path);

    static std::expected<ImageMetadata, std::string> GetImageMetadata(const std::filesystem::path& path);

    static std::expected<std::vector<std::uint8_t>, std::string> GetImageData(const std::filesystem::path& path);

    static std::string MediaTypeToString(MediaType type);

    static bool IsImageFile(const std::filesystem::path& path);
    static bool IsVideoFile(const std::filesystem::path& path);
    static bool IsAudioFile(const std::filesystem::path& path);
    static bool IsModelFile(const std::filesystem::path& path);
    static bool IsTextFile(const std::filesystem::path& path);

  private:
    static std::string GetExtension(const std::filesystem::path& path);
};

} // namespace Core

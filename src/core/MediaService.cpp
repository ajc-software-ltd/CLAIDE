// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MediaService.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/MediaService.hpp"

#include <algorithm>
#include <fstream>
#include <set>

#include <spdlog/spdlog.h>

// Include ImageMagick after standard headers to avoid macro pollution
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <Magick++.h>
#pragma GCC diagnostic pop

namespace Core {

namespace {

const std::set<std::string> kImageExtensions = {".png", ".jpg", ".jpeg", ".bmp", ".gif",  ".tiff", ".tif",
                                                ".ico", ".pcx", ".pnm",  ".xpm", ".webp", ".psd",  ".psb"};

const std::set<std::string> kVideoExtensions = {".mp4", ".webm", ".mkv", ".avi", ".mov", ".wmv", ".flv"};

const std::set<std::string> kAudioExtensions = {".mp3", ".wav", ".ogg", ".flac", ".aac", ".wma"};

const std::set<std::string> kModelExtensions = {".fbx", ".obj", ".gltf", ".glb", ".stl", ".dae"};

const std::set<std::string> kTextExtensions = {".txt", ".md",   ".cpp", ".hpp",  ".c",     ".h",    ".py",  ".js",
                                               ".ts",  ".json", ".xml", ".html", ".css",   ".yaml", ".yml", ".toml",
                                               ".cfg", ".ini",  ".sh",  ".bat",  ".cmake", ".txt"};

bool HasExtension(const std::filesystem::path& path, const std::set<std::string>& extensions) {
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    return extensions.count(ext) > 0;
}

} // namespace

MediaType MediaService::DetectMediaType(const std::filesystem::path& path) {
    if (IsImageFile(path))
        return MediaType::Image;
    if (IsVideoFile(path))
        return MediaType::Video;
    if (IsAudioFile(path))
        return MediaType::Audio;
    if (IsModelFile(path))
        return MediaType::Model;
    if (IsTextFile(path))
        return MediaType::Text;
    return MediaType::Unknown;
}

std::expected<ImageMetadata, std::string> MediaService::GetImageMetadata(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return std::unexpected("File not found: " + path.string());
    }

    try {
        Magick::Image img(path.string());
        ImageMetadata meta;
        meta.width = static_cast<int>(img.columns());
        meta.height = static_cast<int>(img.rows());
        meta.colorDepth = static_cast<int>(img.depth());
        meta.format = img.magick();
        meta.fileSize = std::filesystem::file_size(path);

        spdlog::debug("MediaService::GetImageMetadata: {}x{}, {}-bit, {}", meta.width, meta.height, meta.colorDepth,
                      meta.format);

        return meta;
    } catch (const Magick::Exception& e) {
        spdlog::warn("MediaService::GetImageMetadata: {}", e.what());
        return std::unexpected("Failed to read image: " + std::string(e.what()));
    }
}

std::expected<std::vector<std::uint8_t>, std::string> MediaService::GetImageData(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return std::unexpected("File not found: " + path.string());
    }

    try {
        Magick::Image img(path.string());
        img.type(Magick::TrueColorType);

        Magick::Blob blob;
        img.write(&blob, "RGBA");

        auto data = static_cast<const std::uint8_t*>(blob.data());
        return std::vector<std::uint8_t>(data, data + blob.length());
    } catch (const Magick::Exception& e) {
        spdlog::warn("MediaService::GetImageData: {}", e.what());
        return std::unexpected("Failed to extract image data: " + std::string(e.what()));
    }
}

std::string MediaService::MediaTypeToString(MediaType type) {
    switch (type) {
    case MediaType::Image:
        return "Image";
    case MediaType::Video:
        return "Video";
    case MediaType::Audio:
        return "Audio";
    case MediaType::Model:
        return "Model";
    case MediaType::Text:
        return "Text";
    case MediaType::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

bool MediaService::IsImageFile(const std::filesystem::path& path) {
    return HasExtension(path, kImageExtensions);
}

bool MediaService::IsVideoFile(const std::filesystem::path& path) {
    return HasExtension(path, kVideoExtensions);
}

bool MediaService::IsAudioFile(const std::filesystem::path& path) {
    return HasExtension(path, kAudioExtensions);
}

bool MediaService::IsModelFile(const std::filesystem::path& path) {
    return HasExtension(path, kModelExtensions);
}

bool MediaService::IsTextFile(const std::filesystem::path& path) {
    return HasExtension(path, kTextExtensions);
}

} // namespace Core

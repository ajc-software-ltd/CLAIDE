// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        ThumbnailCache.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "ui/ThumbnailCache.hpp"

#include <wx/dcclient.h>
#include <wx/image.h>
#include <wx/renderer.h>

#include <Magick++.h>

#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>

#include "core/MediaService.hpp"
#include "platform/PlatformPaths.hpp"

namespace Ui {

ThumbnailCache::ThumbnailCache()
    : m_maxMemoryBytes(static_cast<std::size_t>(500) * static_cast<std::size_t>(1024) * static_cast<std::size_t>(1024)),
      m_currentMemory(0) {
    m_cacheDir = GetCacheDir();
    std::filesystem::create_directories(m_cacheDir);
}

wxBitmap ThumbnailCache::GetThumbnail(const std::filesystem::path& path, int size) {
    std::string key = GetCacheKey(path);

    auto it = m_memoryCache.find(key);
    if (it != m_memoryCache.end()) {
        return it->second;
    }

    wxBitmap thumb = GenerateImageThumbnail(path, size);
    if (thumb.IsOk()) {
        std::size_t memSize = static_cast<std::size_t>(thumb.GetWidth()) * static_cast<std::size_t>(thumb.GetHeight()) *
                              static_cast<std::size_t>(4);

        while (m_currentMemory + memSize > m_maxMemoryBytes && !m_memoryCache.empty()) {
            auto oldest = m_memoryCache.begin();
            m_currentMemory -= static_cast<std::size_t>(oldest->second.GetWidth()) *
                               static_cast<std::size_t>(oldest->second.GetHeight()) * static_cast<std::size_t>(4);
            m_memoryCache.erase(oldest);
        }

        m_memoryCache[key] = thumb;
        m_currentMemory += memSize;
    }

    return thumb;
}

bool ThumbnailCache::HasThumbnail(const std::filesystem::path& path) const {
    std::string key = GetCacheKey(path);
    return m_memoryCache.find(key) != m_memoryCache.end();
}

void ThumbnailCache::Clear() {
    m_memoryCache.clear();
    m_currentMemory = 0;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(m_cacheDir)) {
        std::filesystem::remove(entry.path(), ec);
    }
}

wxBitmap ThumbnailCache::GenerateImageThumbnail(const std::filesystem::path& path, int size) {
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec) || ec) {
        return GenerateDefaultThumbnail("folder", size);
    }

    auto mediaType = Core::MediaService::DetectMediaType(path);
    if (mediaType != Core::MediaType::Image) {
        const char* thumbnailType = "file";
        if (mediaType == Core::MediaType::Video) {
            thumbnailType = "video";
        } else if (mediaType == Core::MediaType::Model) {
            thumbnailType = "model";
        }
        return GenerateDefaultThumbnail(thumbnailType, size);
    }

    try {
        Magick::Image img(path.string());
        img.type(Magick::TrueColorType);

        int srcW = static_cast<int>(img.columns());
        int srcH = static_cast<int>(img.rows());

        int thumbW = size;
        int thumbH = size;

        if (srcW > srcH) {
            thumbH = static_cast<int>(static_cast<double>(size) * srcH / srcW);
        } else if (srcH > srcW) {
            thumbW = static_cast<int>(static_cast<double>(size) * srcW / srcH);
        }

        img.resize(Magick::Geometry(thumbW, thumbH));

        Magick::Blob blob;
        img.write(&blob, "RGB");

        // wxImage needs data allocated with new unsigned char[] when staticData=false.
        // Copy blob data into a new[] buffer that wxImage can own and delete.
        std::size_t dataLen = blob.length();
        unsigned char* ownedData = new unsigned char[dataLen];
        std::memcpy(ownedData, blob.data(), dataLen);

        wxImage wxImg(thumbW, thumbH, ownedData, false);

        wxBitmap bmp(wxImg);

        spdlog::debug("ThumbnailCache: generated {}x{} thumbnail for {}", thumbW, thumbH, path.filename().string());
        return bmp;
    } catch (const Magick::Exception& e) {
        spdlog::warn("ThumbnailCache: failed to generate thumbnail for {}: {}", path.filename().string(), e.what());
        return GenerateDefaultThumbnail("image", size);
    } catch (const std::exception& e) {
        spdlog::warn("ThumbnailCache: unexpected error for {}: {}", path.filename().string(), e.what());
        return GenerateDefaultThumbnail("file", size);
    }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
wxBitmap ThumbnailCache::GenerateDefaultThumbnail(const std::string& type, int size) {
    if (type == "folder") {
        auto projectRoot = Platform::GetProjectRoot();
        auto iconPath = projectRoot / "assets" / "icons" / "folder_icon_128x128.png";

        if (std::filesystem::exists(iconPath)) {
            wxImage img(iconPath.string(), wxBITMAP_TYPE_PNG);
            if (img.IsOk()) {
                img.Rescale(size, size, wxIMAGE_QUALITY_HIGH);
                return wxBitmap(img);
            }
        }
    }

    wxImage img(size, size);

    if (type == "image") {
        img.SetRGB(wxRect(0, 0, size, size), 60, 90, 130);
    } else if (type == "video") {
        img.SetRGB(wxRect(0, 0, size, size), 130, 60, 60);
    } else if (type == "model") {
        img.SetRGB(wxRect(0, 0, size, size), 60, 130, 60);
    } else {
        img.SetRGB(wxRect(0, 0, size, size), 100, 100, 100);
    }

    return wxBitmap(img);
}

std::string ThumbnailCache::GetCacheKey(const std::filesystem::path& path) const {
    std::error_code ec;
    auto modTime = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return path.string() + "_0";
    }
    auto epoch = std::chrono::duration_cast<std::chrono::seconds>(modTime.time_since_epoch()).count();
    return path.string() + "_" + std::to_string(epoch);
}

std::filesystem::path ThumbnailCache::GetCacheDir() const {
    auto home = std::getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / ".cache" / "cliade" / "thumbnails";
    }
    return std::filesystem::current_path() / "thumbnails";
}

} // namespace Ui

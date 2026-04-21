// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        ThumbnailCache.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <wx/bitmap.h>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Ui {

class ThumbnailCache
{
  public:
    ThumbnailCache();

    wxBitmap GetThumbnail(const std::filesystem::path& path, int size = 128);
    bool HasThumbnail(const std::filesystem::path& path) const;
    void Clear();

  private:
    wxBitmap GenerateImageThumbnail(const std::filesystem::path& path, int size);
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    wxBitmap GenerateDefaultThumbnail(const std::string& type, int size);

    std::string GetCacheKey(const std::filesystem::path& path) const;
    std::filesystem::path GetCacheDir() const;

    std::unordered_map<std::string, wxBitmap> m_memoryCache;
    std::filesystem::path m_cacheDir;
    std::size_t m_maxMemoryBytes;
    std::size_t m_currentMemory;
};

} // namespace Ui

// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        TileCache.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace Core {

struct TileKey {
    int x;
    int y;
    int level;

    bool operator==(const TileKey& other) const {
        return x == other.x && y == other.y && level == other.level;
    }
};

struct TileData {
    std::vector<std::uint8_t> pixels;
    int width;
    int height;
};

class TileCache {
public:
    TileCache(std::size_t maxMemoryBytes = 512 * 1024 * 1024);

    void SetTile(const TileKey& key, TileData data);
    std::optional<TileData> GetTile(const TileKey& key) const;

    void Clear();
    [[nodiscard]] std::size_t GetMemoryUsage() const;
    [[nodiscard]] std::size_t GetMaxMemory() const;

    void SetSwapDirectory(const std::filesystem::path& dir);

private:
    void EvictOldest();
    void SwapToDisk(const TileKey& key, const TileData& data);
    std::optional<TileData> LoadFromDisk(const TileKey& key) const;

    struct CacheEntry {
        TileData data;
        mutable std::size_t accessOrder;
    };

    std::vector<TileKey> m_keys;
    mutable std::vector<CacheEntry> m_entries;
    std::size_t m_maxMemory;
    std::size_t m_currentMemory;
    mutable std::size_t m_accessCounter;
    std::filesystem::path m_swapDir;
};

} // namespace Core

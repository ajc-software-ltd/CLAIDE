// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        TileCache.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/TileCache.hpp"

#include <algorithm>
#include <fstream>

#include <spdlog/spdlog.h>

namespace Core {

TileCache::TileCache(std::size_t maxMemoryBytes)
    : m_maxMemory(maxMemoryBytes), m_currentMemory(0), m_accessCounter(0) {}

void TileCache::SetTile(const TileKey& key, TileData data) {
    std::size_t dataSize = data.pixels.size();

    if (dataSize > m_maxMemory) {
        spdlog::warn("TileCache::SetTile: tile too large ({} bytes)", dataSize);
        return;
    }

    for (std::size_t i = 0; i < m_keys.size(); ++i) {
        if (m_keys[i] == key) {
            m_currentMemory -= m_entries[i].data.pixels.size();
            m_entries[i].data = std::move(data);
            m_entries[i].accessOrder = m_accessCounter++;
            m_currentMemory += m_entries[i].data.pixels.size();
            return;
        }
    }

    while (m_currentMemory + dataSize > m_maxMemory && !m_keys.empty()) {
        EvictOldest();
    }

    m_keys.push_back(key);
    m_entries.push_back({std::move(data), m_accessCounter++});
    m_currentMemory += m_entries.back().data.pixels.size();

    spdlog::debug("TileCache::SetTile: tile ({},{}) level {}, {} bytes, total {} bytes",
                  key.x, key.y, key.level, dataSize, m_currentMemory);
}

std::optional<TileData> TileCache::GetTile(const TileKey& key) const {
    for (std::size_t i = 0; i < m_keys.size(); ++i) {
        if (m_keys[i] == key) {
            m_entries[i].accessOrder = m_accessCounter++;

            auto diskData = LoadFromDisk(key);
            if (diskData) {
                return diskData;
            }
            return m_entries[i].data;
        }
    }
    return std::nullopt;
}

void TileCache::Clear() {
    m_keys.clear();
    m_entries.clear();
    m_currentMemory = 0;
    spdlog::debug("TileCache::Clear");
}

std::size_t TileCache::GetMemoryUsage() const {
    return m_currentMemory;
}

std::size_t TileCache::GetMaxMemory() const {
    return m_maxMemory;
}

void TileCache::SetSwapDirectory(const std::filesystem::path& dir) {
    m_swapDir = dir;
    std::filesystem::create_directories(dir);
}

void TileCache::EvictOldest() {
    if (m_keys.empty()) return;

    std::size_t oldestIdx = 0;
    std::size_t oldestOrder = m_entries[0].accessOrder;

    for (std::size_t i = 1; i < m_entries.size(); ++i) {
        if (m_entries[i].accessOrder < oldestOrder) {
            oldestOrder = m_entries[i].accessOrder;
            oldestIdx = i;
        }
    }

    std::size_t freed = m_entries[oldestIdx].data.pixels.size();
    SwapToDisk(m_keys[oldestIdx], m_entries[oldestIdx].data);

    m_currentMemory -= freed;
    m_keys.erase(m_keys.begin() + oldestIdx);
    m_entries.erase(m_entries.begin() + oldestIdx);

    spdlog::debug("TileCache::EvictOldest: freed {} bytes, total {} bytes",
                  freed, m_currentMemory);
}

void TileCache::SwapToDisk(const TileKey& key, const TileData& data) {
    if (m_swapDir.empty()) return;

    std::string filename = std::to_string(key.x) + "_" +
                           std::to_string(key.y) + "_" +
                           std::to_string(key.level) + ".tile";
    auto path = m_swapDir / filename;

    std::ofstream file(path, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&data.width), sizeof(int));
        file.write(reinterpret_cast<const char*>(&data.height), sizeof(int));
        file.write(reinterpret_cast<const char*>(data.pixels.data()),
                   static_cast<std::streamsize>(data.pixels.size()));
        spdlog::debug("TileCache::SwapToDisk: {}", path.string());
    }
}

std::optional<TileData> TileCache::LoadFromDisk(const TileKey& key) const {
    if (m_swapDir.empty()) return std::nullopt;

    std::string filename = std::to_string(key.x) + "_" +
                           std::to_string(key.y) + "_" +
                           std::to_string(key.level) + ".tile";
    auto path = m_swapDir / filename;

    if (!std::filesystem::exists(path)) return std::nullopt;

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return std::nullopt;

    TileData data;
    file.read(reinterpret_cast<char*>(&data.width), sizeof(int));
    file.read(reinterpret_cast<char*>(&data.height), sizeof(int));

    std::size_t pixelCount = static_cast<std::size_t>(data.width) *
                             static_cast<std::size_t>(data.height) * 4;
    data.pixels.resize(pixelCount);
    file.read(reinterpret_cast<char*>(data.pixels.data()),
              static_cast<std::streamsize>(pixelCount));

    spdlog::debug("TileCache::LoadFromDisk: {}", path.string());
    return data;
}

} // namespace Core

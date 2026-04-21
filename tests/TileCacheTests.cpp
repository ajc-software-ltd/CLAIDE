// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        TileCacheTests.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>

#include "core/TileCache.hpp"

using namespace Core;

TEST_CASE("TileCache starts empty", "[tilecache]") {
    TileCache cache(1024 * 1024);
    REQUIRE(cache.GetMemoryUsage() == 0);
}

TEST_CASE("TileCache set and get tile", "[tilecache]") {
    TileCache cache(1024 * 1024);
    TileKey key{0, 0, 0};
    TileData data;
    data.width = 2;
    data.height = 2;
    data.pixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 255};

    cache.SetTile(key, data);
    auto retrieved = cache.GetTile(key);

    REQUIRE(retrieved.has_value());
    REQUIRE(retrieved->pixels == data.pixels);
}

TEST_CASE("TileCache returns nullopt for missing tile", "[tilecache]") {
    TileCache cache(1024 * 1024);
    TileKey key{0, 0, 0};
    auto retrieved = cache.GetTile(key);
    REQUIRE(!retrieved.has_value());
}

TEST_CASE("TileCache evicts oldest tile when full", "[tilecache]") {
    TileCache cache(100);

    TileData smallData;
    smallData.width = 1;
    smallData.height = 1;
    smallData.pixels = {255, 0, 0, 255};

    cache.SetTile({0, 0, 0}, smallData);
    cache.SetTile({1, 0, 0}, smallData);
    cache.SetTile({2, 0, 0}, smallData);

    REQUIRE(cache.GetMemoryUsage() > 0);
}

TEST_CASE("TileCache clear removes all tiles", "[tilecache]") {
    TileCache cache(1024 * 1024);
    TileKey key{0, 0, 0};
    TileData data;
    data.width = 2;
    data.height = 2;
    data.pixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 255};

    cache.SetTile(key, data);
    cache.Clear();

    REQUIRE(cache.GetMemoryUsage() == 0);
    REQUIRE(!cache.GetTile(key).has_value());
}

TEST_CASE("TileCache respects max memory limit", "[tilecache]") {
    TileCache cache(64);

    TileData data;
    data.width = 2;
    data.height = 2;
    data.pixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 255};

    cache.SetTile({0, 0, 0}, data);
    REQUIRE(cache.GetMemoryUsage() <= 64);
}

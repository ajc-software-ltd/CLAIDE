// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MipmapTests.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>

#include "core/MipmapGenerator.hpp"

using namespace Core;

TEST_CASE("MipmapGenerator generates correct level count", "[mipmap]") {
    REQUIRE(MipmapGenerator::GetLevelCount(256, 256) == 9);
    REQUIRE(MipmapGenerator::GetLevelCount(1, 1) == 1);
    REQUIRE(MipmapGenerator::GetLevelCount(100, 100) == 7);
}

TEST_CASE("MipmapGenerator generates pyramid", "[mipmap]") {
    std::vector<std::uint8_t> input(256 * 256 * 4, 128);
    auto mipmaps = MipmapGenerator::Generate(input.data(), 256, 256, 4);

    REQUIRE(mipmaps.size() == 9);
    REQUIRE(mipmaps[0].size() == 256 * 256 * 4);
    REQUIRE(mipmaps[1].size() == 128 * 128 * 4);
}

TEST_CASE("MipmapGenerator smallest level is 1x1", "[mipmap]") {
    std::vector<std::uint8_t> input(256 * 256 * 4, 128);
    auto mipmaps = MipmapGenerator::Generate(input.data(), 256, 256, 4);

    REQUIRE(mipmaps.back().size() == 1 * 1 * 4);
}

TEST_CASE("MipmapGenerator GetLevel returns correct data", "[mipmap]") {
    std::vector<std::uint8_t> input(4 * 4 * 4, 200);
    auto mipmaps = MipmapGenerator::Generate(input.data(), 4, 4, 4);

    ImageSize size{};
    auto data = MipmapGenerator::GetLevel(mipmaps, 0, size, 4);

    REQUIRE(data != nullptr);
    REQUIRE(size.width == 4);
    REQUIRE(size.height == 4);
}

TEST_CASE("MipmapGenerator GetLevelCopy returns copy", "[mipmap]") {
    std::vector<std::uint8_t> input(4 * 4 * 4, 200);
    auto mipmaps = MipmapGenerator::Generate(input.data(), 4, 4, 4);

    ImageSize size{};
    auto copy = MipmapGenerator::GetLevelCopy(mipmaps, 0, size, 4);

    REQUIRE(copy.size() == 4 * 4 * 4);
    REQUIRE(size.width == 4);
    REQUIRE(size.height == 4);
}

TEST_CASE("MipmapGenerator handles non-square images", "[mipmap]") {
    std::vector<std::uint8_t> input(100 * 50 * 3, 100);
    auto mipmaps = MipmapGenerator::Generate(input.data(), 100, 50, 3);

    REQUIRE(mipmaps.size() >= 1);
}

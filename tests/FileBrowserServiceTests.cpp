// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileBrowserServiceTests.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "core/FileBrowserService.hpp"

namespace {

std::filesystem::path CreateTempDir() {
    auto base = std::filesystem::temp_directory_path();
    static int counter = 0;
    auto dir = base / ("cliade_filebrowser_test_" + std::to_string(counter++));
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

void WriteFile(const std::filesystem::path& path) {
    std::ofstream out(path);
    out << "test";
}

} // namespace

using namespace Core;

TEST_CASE("ListDirectory includes directories first", "[filebrowser]") {
    auto dir = CreateTempDir();
    auto nested = dir / "folder";
    std::filesystem::create_directories(nested);
    WriteFile(dir / "a.txt");

    auto result = FileBrowserService::ListDirectory(dir, FileBrowserFilter::All);
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 2);
    REQUIRE((*result)[0].isDirectory);
    REQUIRE((*result)[0].path.filename() == "folder");

    std::filesystem::remove_all(dir);
}

TEST_CASE("ListDirectory image filter only returns image files", "[filebrowser]") {
    auto dir = CreateTempDir();
    WriteFile(dir / "img.png");
    WriteFile(dir / "code.cpp");

    auto result = FileBrowserService::ListDirectory(dir, FileBrowserFilter::Images);
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 1);
    REQUIRE((*result)[0].path.filename() == "img.png");

    std::filesystem::remove_all(dir);
}

TEST_CASE("IsDirectoryPath validates directories", "[filebrowser]") {
    auto dir = CreateTempDir();
    WriteFile(dir / "file.txt");

    REQUIRE(FileBrowserService::IsDirectoryPath(dir));
    REQUIRE(!FileBrowserService::IsDirectoryPath(dir / "file.txt"));

    std::filesystem::remove_all(dir);
}

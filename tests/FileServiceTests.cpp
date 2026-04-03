// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileServiceTests.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "core/FileService.hpp"

using namespace Core;

namespace {

std::filesystem::path GetTempDir() {
    return std::filesystem::temp_directory_path() / "cliade_tests";
}

void EnsureTempDir() {
    std::filesystem::create_directories(GetTempDir());
}

void CleanupTempDir() {
    std::error_code ec;
    std::filesystem::remove_all(GetTempDir(), ec);
}

} // namespace

TEST_CASE("LoadFile loads a valid UTF-8 file", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_utf8.txt";

    std::ofstream file(path, std::ios::binary);
    file << "Hello, World!";
    file.close();

    auto result = FileService::LoadFile(path);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "Hello, World!");

    CleanupTempDir();
}

TEST_CASE("LoadFile loads a UTF-8 BOM file", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_bom.txt";

    std::ofstream file(path, std::ios::binary);
    file << static_cast<char>(0xEF) << static_cast<char>(0xBB)
         << static_cast<char>(0xBF);
    file << "BOM Test";
    file.close();

    auto result = FileService::LoadFile(path);
    REQUIRE(result.has_value());
    REQUIRE(result->text == "BOM Test");
    REQUIRE(result->detectedEncoding == TextEncoding::Utf8Bom);
    REQUIRE(result->hadBom);

    CleanupTempDir();
}

TEST_CASE("LoadFile returns error for non-existent file", "[fileservice]") {
    auto path = GetTempDir() / "nonexistent.txt";
    auto result = FileService::LoadFile(path);
    REQUIRE(!result.has_value());
}

TEST_CASE("SaveFile writes content to disk", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_save.txt";

    auto result = FileService::SaveFile(path, "Saved content",
                                        TextEncoding::Utf8);
    REQUIRE(result.has_value());

    std::ifstream file(path, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    REQUIRE(content == "Saved content");

    CleanupTempDir();
}

TEST_CASE("SaveFile writes UTF-8 BOM content", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_save_bom.txt";

    auto result = FileService::SaveFile(path, "BOM Save",
                                        TextEncoding::Utf8Bom);
    REQUIRE(result.has_value());

    std::ifstream file(path, std::ios::binary);
    std::vector<char> data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    REQUIRE(static_cast<unsigned char>(data[0]) == 0xEF);
    REQUIRE(static_cast<unsigned char>(data[1]) == 0xBB);
    REQUIRE(static_cast<unsigned char>(data[2]) == 0xBF);

    CleanupTempDir();
}

TEST_CASE("SaveFile writes UTF-16 LE content", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_save_utf16le.txt";

    auto result = FileService::SaveFile(path, "UTF16",
                                        TextEncoding::Utf16Le);
    REQUIRE(result.has_value());

    std::ifstream file(path, std::ios::binary);
    std::vector<char> data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    REQUIRE(static_cast<unsigned char>(data[0]) == 0xFF);
    REQUIRE(static_cast<unsigned char>(data[1]) == 0xFE);

    CleanupTempDir();
}

TEST_CASE("DeleteFile removes an existing file", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_delete.txt";

    std::ofstream file(path);
    file << "delete me";
    file.close();

    auto result = FileService::DeleteFile(path);
    REQUIRE(result.has_value());
    REQUIRE(!std::filesystem::exists(path));

    CleanupTempDir();
}

TEST_CASE("DeleteFile returns error for non-existent file", "[fileservice]") {
    auto path = GetTempDir() / "nonexistent.txt";
    auto result = FileService::DeleteFile(path);
    REQUIRE(!result.has_value());
}

TEST_CASE("FileExists returns correct values", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_exists.txt";

    std::ofstream file(path);
    file << "exists";
    file.close();

    REQUIRE(FileService::FileExists(path));

    std::filesystem::remove(path);
    REQUIRE(!FileService::FileExists(path));

    CleanupTempDir();
}

TEST_CASE("Safe save overwrites existing file", "[fileservice]") {
    EnsureTempDir();
    auto path = GetTempDir() / "test_overwrite.txt";

    auto first = FileService::SaveFile(path, "First", TextEncoding::Utf8);
    REQUIRE(first.has_value());

    auto second = FileService::SaveFile(path, "Second", TextEncoding::Utf8);
    REQUIRE(second.has_value());

    auto loaded = FileService::LoadFile(path);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->text == "Second");

    CleanupTempDir();
}

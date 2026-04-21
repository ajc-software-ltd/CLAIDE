// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileSystemService.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/FileSystemService.hpp"

namespace Core {

bool FileSystemService::PathExists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec) && !ec;
}

bool FileSystemService::IsRegularFile(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec) && !ec;
}

void FileSystemService::EnsureDirectory(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
}

void FileSystemService::RemoveDirectoryContents(const std::filesystem::path& path) {
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
        std::filesystem::remove(entry.path(), ec);
    }
}

} // namespace Core

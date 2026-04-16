// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileBrowserService.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace Core {

enum class FileBrowserFilter {
    All = 0,
    Images = 1,
    Video = 2,
    Models = 3
};

struct FileBrowserEntry {
    std::filesystem::path path;
    bool isDirectory = false;
};

class FileBrowserService {
public:
    static std::expected<std::vector<FileBrowserEntry>, std::string> ListDirectory(
        const std::filesystem::path& path, FileBrowserFilter filter);

    static bool IsDirectoryPath(const std::filesystem::path& path);
};

} // namespace Core

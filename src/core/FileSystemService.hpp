// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileSystemService.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <filesystem>

namespace Core {

class FileSystemService
{
  public:
    static bool PathExists(const std::filesystem::path& path);
    static bool IsRegularFile(const std::filesystem::path& path);
    static void EnsureDirectory(const std::filesystem::path& path);
    static void RemoveDirectoryContents(const std::filesystem::path& path);
};

} // namespace Core

// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileBrowserService.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/FileBrowserService.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

#include "core/MediaService.hpp"

namespace Core {

namespace {

bool MatchesFilter(const std::filesystem::path& path, FileBrowserFilter filter) {
    if (filter == FileBrowserFilter::All) {
        return true;
    }

    auto mediaType = MediaService::DetectMediaType(path);
    switch (filter) {
    case FileBrowserFilter::Images:
        return mediaType == MediaType::Image;
    case FileBrowserFilter::Video:
        return mediaType == MediaType::Video;
    case FileBrowserFilter::Models:
        return mediaType == MediaType::Model;
    case FileBrowserFilter::All:
    default:
        return true;
    }
}

} // namespace

std::expected<std::vector<FileBrowserEntry>, std::string> FileBrowserService::ListDirectory(
    const std::filesystem::path& path,
    FileBrowserFilter filter) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        return std::unexpected("Directory does not exist: " + path.string());
    }
    if (!std::filesystem::is_directory(path, ec) || ec) {
        return std::unexpected("Path is not a directory: " + path.string());
    }

    std::vector<FileBrowserEntry> items;
    auto dirIter = std::filesystem::directory_iterator(
        path, std::filesystem::directory_options::skip_permission_denied, ec);
    if (ec) {
        return std::unexpected("Cannot iterate directory: " + ec.message());
    }

    for (const auto& entry : dirIter) {
        std::error_code entryEc;
        std::filesystem::path itemPath = entry.path();

        auto name = itemPath.filename().string();
        if (!name.empty() && name[0] == '.') {
            continue;
        }

        if (entry.is_directory(entryEc) && !entryEc) {
            items.push_back({std::move(itemPath), true});
            continue;
        }

        if (entry.is_regular_file(entryEc) && !entryEc && MatchesFilter(itemPath, filter)) {
            items.push_back({std::move(itemPath), false});
        }
    }

    std::sort(items.begin(), items.end(),
              [](const FileBrowserEntry& a, const FileBrowserEntry& b) {
                  if (a.isDirectory != b.isDirectory) {
                      return a.isDirectory;
                  }
                  return a.path.filename().string() < b.path.filename().string();
              });

    return items;
}

bool FileBrowserService::IsDirectory(const std::filesystem::path& path) {
    std::error_code ec;
    auto result = std::filesystem::is_directory(path, ec);
    if (ec) {
        spdlog::warn("FileBrowserService::IsDirectory failed: {}", ec.message());
        return false;
    }
    return result;
}

bool FileBrowserService::IsDirectoryPath(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        return false;
    }
    return std::filesystem::is_directory(path, ec) && !ec;
}

} // namespace Core

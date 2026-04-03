// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileService.hpp
// Project:     CLIADE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

#include "core/Encoding.hpp"

namespace Core {

class FileService {
public:
    static std::expected<DecodeResult, std::string> LoadFile(
        const std::filesystem::path& path);

    static std::expected<void, std::string> SaveFile(
        const std::filesystem::path& path,
        std::string_view content,
        TextEncoding encoding);

    static std::expected<void, std::string> DeleteFile(
        const std::filesystem::path& path);

    [[nodiscard]] static bool FileExists(const std::filesystem::path& path);

private:
    static std::expected<void, std::string> SafeSave(
        const std::filesystem::path& path,
        std::string_view content,
        TextEncoding encoding);
};

} // namespace Core

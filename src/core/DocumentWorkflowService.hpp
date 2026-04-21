// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        DocumentWorkflowService.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

#include "core/Encoding.hpp"

namespace Core {

struct LoadedDocumentData
{
    std::string content;
    TextEncoding encoding = TextEncoding::Utf8;
};

class DocumentWorkflowService
{
  public:
    std::expected<LoadedDocumentData, std::string> LoadTextDocument(const std::filesystem::path& path) const;
    std::expected<void, std::string> SaveTextDocument(const std::filesystem::path& path, std::string_view content,
                                                      TextEncoding encoding) const;
    std::expected<void, std::string> DeleteDocumentFile(const std::filesystem::path& path) const;
    bool PathExists(const std::filesystem::path& path) const;
};

} // namespace Core

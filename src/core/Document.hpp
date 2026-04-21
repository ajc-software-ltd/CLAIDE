// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        Document.hpp
// Project:     CLAIDE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "core/Encoding.hpp"

namespace Core {

class Document
{
  public:
    Document();

    [[nodiscard]] std::string_view GetContent() const;
    void SetContent(std::string_view content);

    [[nodiscard]] std::optional<std::filesystem::path> GetFilePath() const;
    void SetFilePath(std::filesystem::path path);

    [[nodiscard]] bool IsModified() const;
    void SetModified(bool modified);

    [[nodiscard]] TextEncoding GetEncoding() const;
    void SetEncoding(TextEncoding encoding);

    [[nodiscard]] bool IsUntitled() const;

    void Clear();

    [[nodiscard]] std::string GetDisplayName() const;

  private:
    std::string m_content;
    std::optional<std::filesystem::path> m_filePath;
    bool m_modified;
    TextEncoding m_encoding;
};

} // namespace Core

// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MinidocxAdapter.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <string>

namespace md {
class Document;
namespace inspection {
struct DocumentStats;
} // namespace inspection
} // namespace md

namespace Core::Office {

struct OfficeDocumentSummary
{
    size_t sectionCount = 0;
    size_t blockCount = 0;
    size_t paragraphCount = 0;
    size_t runCount = 0;
    size_t tableCount = 0;
    size_t cellCount = 0;
    size_t pictureCount = 0;
};

class MinidocxAdapter
{
  public:
    std::expected<void, std::string> Open(const std::filesystem::path& path);
    std::expected<void, std::string> Save(const std::filesystem::path& path) const;
    std::expected<std::string, std::string> ExtractVisibleText() const;
    std::expected<OfficeDocumentSummary, std::string> Summarize() const;

  private:
    std::expected<void, std::string> EnsureLoaded() const;
    static OfficeDocumentSummary ToSummary(const md::inspection::DocumentStats& stats);

    std::unique_ptr<md::Document> m_document;
};

} // namespace Core::Office

// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        OfficeDocumentService.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <filesystem>
#include <string>

#include "core/office/MinidocxAdapter.hpp"

namespace Core::Office {

struct OpenOfficeDocumentResult
{
    std::filesystem::path path;
    std::string visibleText;
    OfficeDocumentSummary summary;
};

struct SaveOfficeDocumentResult
{
    std::filesystem::path path;
};

class OfficeDocumentService
{
  public:
    std::expected<OpenOfficeDocumentResult, std::string> OpenDocx(const std::filesystem::path& path);
    std::expected<SaveOfficeDocumentResult, std::string> SaveDocx(const std::filesystem::path& path);

  private:
    MinidocxAdapter m_minidocxAdapter;
};

} // namespace Core::Office

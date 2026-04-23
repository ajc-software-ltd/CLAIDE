// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        OfficeLayoutBridge.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <expected>
#include <string>
#include <vector>

#include "core/office/OfficeDocumentService.hpp"

namespace Core::Office {

struct OfficeLayoutSpan
{
    size_t startOffset = 0;
    size_t length = 0;
    std::string text;
};

struct OfficeLayoutParagraph
{
    size_t paragraphIndex = 0;
    size_t startOffset = 0;
    size_t length = 0;
    std::string text;
    std::vector<OfficeLayoutSpan> spans;
};

struct OfficeLayoutSnapshot
{
    std::vector<OfficeLayoutParagraph> paragraphs;
    size_t paragraphCount = 0;
    size_t totalTextLength = 0;
};

class OfficeLayoutBridge
{
  public:
    std::expected<OfficeLayoutSnapshot, std::string>
    BuildSnapshot(const OpenOfficeDocumentResult& openResult) const;
};

} // namespace Core::Office

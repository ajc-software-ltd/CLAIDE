// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        OfficeDocumentService.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/office/OfficeDocumentService.hpp"

namespace Core::Office {

std::expected<OpenOfficeDocumentResult, std::string> OfficeDocumentService::OpenDocx(const std::filesystem::path& path) {
    auto openResult = m_minidocxAdapter.Open(path);
    if (!openResult) {
        return std::unexpected(openResult.error());
    }

    auto textResult = m_minidocxAdapter.ExtractVisibleText();
    if (!textResult) {
        return std::unexpected(textResult.error());
    }

    auto summaryResult = m_minidocxAdapter.Summarize();
    if (!summaryResult) {
        return std::unexpected(summaryResult.error());
    }

    return OpenOfficeDocumentResult{.path = path, .visibleText = std::move(*textResult), .summary = *summaryResult};
}

std::expected<SaveOfficeDocumentResult, std::string> OfficeDocumentService::SaveDocx(const std::filesystem::path& path) {
    auto saveResult = m_minidocxAdapter.Save(path);
    if (!saveResult) {
        return std::unexpected(saveResult.error());
    }

    return SaveOfficeDocumentResult{.path = path};
}

} // namespace Core::Office

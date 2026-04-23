// ============================================================================
// CLAIDE - Cross-platform Text Editor
// ============================================================================
// File:        MinidocxAdapter.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/office/MinidocxAdapter.hpp"

#include <exception>

#include <minidocx/inspection/semantic.hpp>
#include <minidocx/word/main/document.hpp>

namespace Core::Office {

std::expected<void, std::string> MinidocxAdapter::Open(const std::filesystem::path& path) {
    try {
        auto loaded = std::make_unique<md::Document>();
        loaded->load(path.string());
        m_document = std::move(loaded);
        return {};
    } catch (const std::exception& ex) {
        return std::unexpected(std::string("MinidocxAdapter::Open failed: ") + ex.what());
    }
}

std::expected<void, std::string> MinidocxAdapter::Save(const std::filesystem::path& path) const {
    auto loaded = EnsureLoaded();
    if (!loaded) {
        return std::unexpected(loaded.error());
    }

    try {
        m_document->saveAs(path.string());
        return {};
    } catch (const std::exception& ex) {
        return std::unexpected(std::string("MinidocxAdapter::Save failed: ") + ex.what());
    }
}

std::expected<std::string, std::string> MinidocxAdapter::ExtractVisibleText() const {
    auto loaded = EnsureLoaded();
    if (!loaded) {
        return std::unexpected(loaded.error());
    }

    try {
        return md::inspection::extractVisibleText(*m_document);
    } catch (const std::exception& ex) {
        return std::unexpected(std::string("MinidocxAdapter::ExtractVisibleText failed: ") + ex.what());
    }
}

std::expected<OfficeDocumentSummary, std::string> MinidocxAdapter::Summarize() const {
    auto loaded = EnsureLoaded();
    if (!loaded) {
        return std::unexpected(loaded.error());
    }

    try {
        return ToSummary(md::inspection::summarize(*m_document));
    } catch (const std::exception& ex) {
        return std::unexpected(std::string("MinidocxAdapter::Summarize failed: ") + ex.what());
    }
}

std::expected<void, std::string> MinidocxAdapter::EnsureLoaded() const {
    if (!m_document) {
        return std::unexpected("MinidocxAdapter: no document loaded");
    }
    return {};
}

OfficeDocumentSummary MinidocxAdapter::ToSummary(const md::inspection::DocumentStats& stats) {
    return OfficeDocumentSummary{.sectionCount = stats.sectionCount,
                                 .blockCount = stats.blockCount,
                                 .paragraphCount = stats.paragraphCount,
                                 .runCount = stats.runCount,
                                 .tableCount = stats.tableCount,
                                 .cellCount = stats.cellCount,
                                 .pictureCount = stats.pictureCount};
}

} // namespace Core::Office

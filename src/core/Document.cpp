// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        Document.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/Document.hpp"

#include <spdlog/spdlog.h>

namespace Core {

Document::Document() : m_modified(false), m_encoding(TextEncoding::Utf8) {
}

std::string_view Document::GetContent() const {
    return m_content;
}

void Document::SetContent(std::string_view content) {
    m_content = std::string(content);
    m_modified = false;
    spdlog::debug("Document::SetContent: {} bytes", m_content.size());
}

std::optional<std::filesystem::path> Document::GetFilePath() const {
    return m_filePath;
}

void Document::SetFilePath(std::filesystem::path path) {
    m_filePath = std::move(path);
    spdlog::debug("Document::SetFilePath: {}", m_filePath->string());
}

bool Document::IsModified() const {
    return m_modified;
}

void Document::SetModified(bool modified) {
    m_modified = modified;
}

TextEncoding Document::GetEncoding() const {
    return m_encoding;
}

void Document::SetEncoding(TextEncoding encoding) {
    m_encoding = encoding;
    spdlog::debug("Document::SetEncoding: {}", Encoding::EncodingName(encoding));
}

bool Document::IsUntitled() const {
    return !m_filePath.has_value();
}

void Document::Clear() {
    m_content.clear();
    m_filePath.reset();
    m_modified = false;
    m_encoding = TextEncoding::Utf8;
    spdlog::debug("Document::Clear");
}

std::string Document::GetDisplayName() const {
    if (m_filePath.has_value()) {
        return m_filePath->filename().string();
    }
    return "Untitled";
}

} // namespace Core

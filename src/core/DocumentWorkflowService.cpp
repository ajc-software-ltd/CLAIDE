// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        DocumentWorkflowService.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/DocumentWorkflowService.hpp"

#include "core/FileService.hpp"

namespace Core {

std::expected<LoadedDocumentData, std::string>
DocumentWorkflowService::LoadTextDocument(const std::filesystem::path& path) const {
    auto result = FileService::LoadFile(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    return LoadedDocumentData{.content = std::string(result->text), .encoding = result->detectedEncoding};
}

std::expected<void, std::string> DocumentWorkflowService::SaveTextDocument(const std::filesystem::path& path,
                                                                           std::string_view content,
                                                                           TextEncoding encoding) const {
    auto saveResult = FileService::SaveFile(path, content, encoding);
    if (!saveResult) {
        return std::unexpected(saveResult.error());
    }
    return {};
}

std::expected<void, std::string> DocumentWorkflowService::DeleteDocumentFile(const std::filesystem::path& path) const {
    auto deleteResult = FileService::DeleteFile(path);
    if (!deleteResult) {
        return std::unexpected(deleteResult.error());
    }
    return {};
}

bool DocumentWorkflowService::PathExists(const std::filesystem::path& path) const {
    return std::filesystem::exists(path);
}

} // namespace Core

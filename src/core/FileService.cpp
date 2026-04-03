// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        FileService.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/FileService.hpp"

#include <fstream>
#include <random>

#include <spdlog/spdlog.h>

namespace Core {

std::expected<DecodeResult, std::string> FileService::LoadFile(
    const std::filesystem::path& path) {
    spdlog::info("FileService::LoadFile: {}", path.string());

    if (!std::filesystem::exists(path)) {
        spdlog::error("FileService::LoadFile: file not found: {}",
                      path.string());
        return std::unexpected("File not found: " + path.string());
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        spdlog::error("FileService::LoadFile: cannot open file: {}",
                      path.string());
        return std::unexpected("Cannot open file: " + path.string());
    }

    std::vector<std::uint8_t> data(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    if (file.bad()) {
        spdlog::error("FileService::LoadFile: read error: {}", path.string());
        return std::unexpected("Error reading file: " + path.string());
    }

    spdlog::debug("FileService::LoadFile: read {} bytes", data.size());

    auto decodeResult = Encoding::Decode(data);
    if (!decodeResult) {
        spdlog::warn("FileService::LoadFile: decode warning: {}",
                     decodeResult.error());
        return std::unexpected(decodeResult.error());
    }

    spdlog::info("FileService::LoadFile: loaded {} bytes, encoding: {}",
                 data.size(),
                 Encoding::EncodingName(decodeResult->detectedEncoding));

    return *decodeResult;
}

std::expected<void, std::string> FileService::SafeSave(
    const std::filesystem::path& path,
    std::string_view content,
    TextEncoding encoding) {
    auto parentDir = path.parent_path();
    if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
        spdlog::error("FileService::SafeSave: directory does not exist: {}",
                      parentDir.string());
        return std::unexpected("Directory does not exist: " +
                               parentDir.string());
    }

    std::string tempName = path.filename().string() + ".tmp.";
    std::string suffix;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);
    suffix = std::to_string(dis(gen));
    tempName += suffix;

    auto tempPath = parentDir / tempName;

    auto encodeResult = Encoding::Encode(content, encoding);
    if (!encodeResult) {
        spdlog::error("FileService::SafeSave: encode failed: {}",
                      encodeResult.error());
        return std::unexpected("Encoding failed: " + encodeResult.error());
    }

    std::ofstream outFile(tempPath, std::ios::binary);
    if (!outFile.is_open()) {
        spdlog::error("FileService::SafeSave: cannot create temp file: {}",
                      tempPath.string());
        return std::unexpected("Cannot create temporary file");
    }

    outFile.write(
        reinterpret_cast<const char*>(encodeResult->bytes.data()),
        static_cast<std::streamsize>(encodeResult->bytes.size()));

    if (outFile.bad()) {
        outFile.close();
        std::filesystem::remove(tempPath);
        spdlog::error("FileService::SafeSave: write error to temp file");
        return std::unexpected("Error writing to temporary file");
    }

    outFile.flush();
    outFile.close();

    try {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
        std::filesystem::rename(tempPath, path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::filesystem::remove(tempPath);
        spdlog::error("FileService::SafeSave: atomic replace failed: {}",
                      e.what());
        return std::unexpected("Failed to replace file: " + std::string(e.what()));
    }

    spdlog::info("FileService::SafeSave: saved successfully: {}",
                 path.string());
    return {};
}

std::expected<void, std::string> FileService::SaveFile(
    const std::filesystem::path& path,
    std::string_view content,
    TextEncoding encoding) {
    spdlog::info("FileService::SaveFile: {}", path.string());
    return SafeSave(path, content, encoding);
}

std::expected<void, std::string> FileService::DeleteFile(
    const std::filesystem::path& path) {
    spdlog::info("FileService::DeleteFile: {}", path.string());

    if (!std::filesystem::exists(path)) {
        spdlog::warn("FileService::DeleteFile: file does not exist: {}",
                     path.string());
        return std::unexpected("File does not exist: " + path.string());
    }

    std::error_code ec;
    std::filesystem::remove(path, ec);
    if (ec) {
        spdlog::error("FileService::DeleteFile: remove failed: {}",
                      ec.message());
        return std::unexpected("Failed to delete file: " + ec.message());
    }

    spdlog::info("FileService::DeleteFile: deleted successfully: {}",
                 path.string());
    return {};
}

bool FileService::FileExists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

} // namespace Core

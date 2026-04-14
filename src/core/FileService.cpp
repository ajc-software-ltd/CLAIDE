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

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Core {

namespace {

std::expected<void, std::string> SyncFileToDisk(const std::filesystem::path& path) {
#ifdef _WIN32
    HANDLE fileHandle = CreateFileW(path.wstring().c_str(),
                                    GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    nullptr,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        return std::unexpected("Failed to open temp file for sync");
    }

    if (!FlushFileBuffers(fileHandle)) {
        CloseHandle(fileHandle);
        return std::unexpected("Failed to flush temp file to disk");
    }

    CloseHandle(fileHandle);
    return {};
#else
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        return std::unexpected("Failed to open temp file for sync");
    }
    if (fsync(fd) != 0) {
        close(fd);
        return std::unexpected("Failed to flush temp file to disk");
    }
    close(fd);
    return {};
#endif
}

std::expected<void, std::string> ReplaceFileAtomically(const std::filesystem::path& from,
                                                       const std::filesystem::path& to) {
#ifdef _WIN32
    if (!MoveFileExW(from.wstring().c_str(),
                     to.wstring().c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        return std::unexpected("Failed to atomically replace file");
    }
    return {};
#else
    std::error_code ec;
    std::filesystem::rename(from, to, ec);
    if (ec) {
        return std::unexpected("Failed to atomically replace file: " + ec.message());
    }
    return {};
#endif
}

} // namespace

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

    auto syncResult = SyncFileToDisk(tempPath);
    if (!syncResult) {
        std::filesystem::remove(tempPath);
        spdlog::error("FileService::SafeSave: {}", syncResult.error());
        return std::unexpected(syncResult.error());
    }

    auto replaceResult = ReplaceFileAtomically(tempPath, path);
    if (!replaceResult) {
        std::filesystem::remove(tempPath);
        spdlog::error("FileService::SafeSave: {}", replaceResult.error());
        return std::unexpected(replaceResult.error());
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

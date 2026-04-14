// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        PlatformPaths.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "platform/PlatformPaths.hpp"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#include <fileapi.h>
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace Platform {

std::filesystem::path GetAppDataDir() {
#ifdef _WIN32
    auto appData = std::getenv("APPDATA");
    if (appData) {
        return std::filesystem::path(appData) / "CLIADE";
    }
    return std::filesystem::current_path();
#else
    auto xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg) {
        return std::filesystem::path(xdg) / "cliade";
    }
    auto home = std::getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / ".config" / "cliade";
    }
    return std::filesystem::current_path();
#endif
}

std::filesystem::path GetProjectRoot() {
    std::filesystem::path exePath;

#ifdef _WIN32
    std::vector<wchar_t> buffer(MAX_PATH, L'\0');
    DWORD len = GetModuleFileNameW(nullptr, buffer.data(),
                                   static_cast<DWORD>(buffer.size()));
    if (len > 0) {
        exePath = std::filesystem::path(std::wstring(buffer.data(), len));
    }
#else
    std::vector<char> buffer(PATH_MAX, '\0');
    auto len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (len > 0) {
        buffer[static_cast<std::size_t>(len)] = '\0';
        exePath = std::filesystem::path(buffer.data());
    }
#endif

    std::error_code ec;
    if (!exePath.empty()) {
        exePath = std::filesystem::weakly_canonical(exePath, ec);
    }

    if (exePath.empty() || ec) {
        auto cwd = std::filesystem::current_path(ec);
        if (ec) {
            return std::filesystem::path(".");
        }
        if (std::filesystem::exists(cwd / "CMakeLists.txt")) {
            return cwd;
        }
        return cwd.parent_path();
    }

    auto projectRoot = exePath.parent_path().parent_path();

    if (std::filesystem::exists(projectRoot / "CMakeLists.txt")) {
        return projectRoot;
    }

    return exePath.parent_path();
}

std::filesystem::path GetLogFilePath() {
#ifndef NDEBUG
    auto projectRoot = GetProjectRoot();
    return projectRoot / "logs" / "notepad.log";
#else
    return GetAppDataDir() / "notepad.log";
#endif
}

} // namespace Platform

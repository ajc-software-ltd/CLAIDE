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

#ifdef _WIN32
#include <windows.h>
#else
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
    auto exePath = std::filesystem::canonical("/proc/self/exe");
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

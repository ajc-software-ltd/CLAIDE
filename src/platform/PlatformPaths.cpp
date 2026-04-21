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

#ifdef _WIN32
#include <fileapi.h>
#include <windows.h>
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
    auto configuredRoot = std::getenv("CLIADE_PROJECT_ROOT");
    if (configuredRoot != nullptr) {
        std::error_code ec;
        auto configured = std::filesystem::weakly_canonical(configuredRoot, ec);
        if (!ec && std::filesystem::exists(configured / "CMakeLists.txt")) {
            return configured;
        }
    }

    std::error_code ec;
    auto current = std::filesystem::current_path(ec);
    if (ec) {
        return std::filesystem::path(".");
    }

    auto candidate = std::filesystem::weakly_canonical(current, ec);
    if (ec) {
        candidate = current;
    }

    while (!candidate.empty()) {
        if (std::filesystem::exists(candidate / "CMakeLists.txt")) {
            return candidate;
        }

        auto parent = candidate.parent_path();
        if (parent == candidate) {
            break;
        }
        candidate = parent;
    }

    return current;
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

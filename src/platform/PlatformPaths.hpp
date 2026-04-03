// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        PlatformPaths.hpp
// Project:     CLIADE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <filesystem>

namespace Platform {

std::filesystem::path GetAppDataDir();

std::filesystem::path GetLogFilePath();

std::filesystem::path GetProjectRoot();

} // namespace Platform

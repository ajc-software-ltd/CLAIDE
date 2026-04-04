// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        CrashHandler.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <filesystem>
#include <string>

namespace Core {

class CrashHandler {
public:
    static void Initialize(const std::filesystem::path& logDir);

private:
#ifdef __linux__
    static void HandleSignal(int signal);
    static void WriteCrashDump(int signal, void* context = nullptr);
#endif

    static std::filesystem::path s_crashDir;
    static std::filesystem::path s_logDir;
};

} // namespace Core

// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        CrashHandler.hpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

#include <csignal>
#include <filesystem>
#include <string>

namespace Core {

class CrashHandler {
public:
    static void Initialize(const std::filesystem::path& logDir);

private:
#ifdef __linux__
    static void HandleSignal(int signal, siginfo_t* info, void* context);
    static void WriteCrashDump(int signal);
    static void HandleTerminate();
#endif

    static std::filesystem::path s_crashDir;
    static std::filesystem::path s_logDir;
    static volatile sig_atomic_t s_inHandler;
};

} // namespace Core

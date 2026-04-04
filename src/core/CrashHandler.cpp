// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        CrashHandler.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include "core/CrashHandler.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#ifdef __linux__
#include <csignal>
#include <cxxabi.h>
#include <execinfo.h>
#include <unistd.h>
#endif

#include <spdlog/spdlog.h>

namespace Core {

std::filesystem::path CrashHandler::s_crashDir;
std::filesystem::path CrashHandler::s_logDir;

void CrashHandler::Initialize(const std::filesystem::path& logDir) {
    s_logDir = logDir;
    s_crashDir = logDir / "crashes";
    std::filesystem::create_directories(s_crashDir);

#ifdef __linux__
    struct sigaction sa;
    sa.sa_handler = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;

    sigaction(SIGSEGV, &sa, nullptr);  // Segmentation fault
    sigaction(SIGABRT, &sa, nullptr);  // Abort
    sigaction(SIGFPE, &sa, nullptr);   // Floating point exception
    sigaction(SIGILL, &sa, nullptr);   // Illegal instruction
    sigaction(SIGBUS, &sa, nullptr);   // Bus error
    sigaction(SIGSYS, &sa, nullptr);   // Bad system call

    spdlog::info("CrashHandler: signal handlers installed");
#endif
}

#ifdef __linux__

static const char* SignalName(int signal) {
    switch (signal) {
    case SIGSEGV: return "SIGSEGV (Segmentation fault)";
    case SIGABRT: return "SIGABRT (Abort)";
    case SIGFPE:  return "SIGFPE (Floating point exception)";
    case SIGILL:  return "SIGILL (Illegal instruction)";
    case SIGBUS:  return "SIGBUS (Bus error)";
    case SIGSYS:  return "SIGSYS (Bad system call)";
    default:      return "Unknown signal";
    }
}

void CrashHandler::WriteCrashDump(int signal, void* /*context*/) {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&timeT);
    char timeBuf[64];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", &tm);

    std::string crashFile = std::string("crash_") + timeBuf + ".dump";
    auto crashPath = s_crashDir / crashFile;

    std::ofstream dump(crashPath);
    if (!dump.is_open()) return;

    // Signal info
    dump << "=== CLIADE Crash Dump ===" << std::endl;
    dump << "Signal: " << SignalName(signal) << std::endl;
    dump << "Time: " << timeBuf << std::endl;
    dump << "PID: " << getpid() << std::endl;
    dump << std::endl;

    // Stack trace
    void* buffer[128];
    int nptrs = backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));
    char** symbols = backtrace_symbols(buffer, nptrs);

    dump << "=== Stack Trace ===" << std::endl;
    for (int i = 0; i < nptrs; ++i) {
        // Try to demangle C++ symbols
        std::string rawSymbol(symbols[i]);

        // Format: module(mangledSymbol+offset) [address]
        auto start = rawSymbol.find('(');
        auto end = rawSymbol.find('+', start != std::string::npos ? start : 0);

        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string mangled = rawSymbol.substr(start + 1, end - start - 1);
            int status = 0;
            char* demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
            if (status == 0 && demangled) {
                dump << "  #" << i << " " << rawSymbol.substr(0, start + 1)
                     << demangled << rawSymbol.substr(end) << std::endl;
                free(demangled);
            } else {
                dump << "  #" << i << " " << symbols[i] << std::endl;
            }
        } else {
            dump << "  #" << i << " " << symbols[i] << std::endl;
        }
    }

    if (symbols) free(symbols);

    dump << std::endl;
    dump << "=== End Crash Dump ===" << std::endl;
    dump.flush();
    dump.close();

    // Also log to spdlog (may not work if heap is corrupted, but try)
    try {
        spdlog::critical("CRASH: {} — dump written to {}", SignalName(signal), crashPath.string());
        spdlog::critical("Stack trace (top 10):");
        void* buf[10];
        int n = backtrace(buf, 10);
        char** syms = backtrace_symbols(buf, n);
        for (int i = 0; i < n; ++i) {
            spdlog::critical("  #{} {}", i, syms[i]);
        }
        if (syms) free(syms);
    } catch (...) {
        // spdlog may be corrupted, write directly to stderr
        fprintf(stderr, "CRASH: %s — dump: %s\n", SignalName(signal), crashPath.c_str());
    }
}

void CrashHandler::HandleSignal(int sig) {
    WriteCrashDump(sig);

    // Re-raise signal to generate core dump
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

#endif

} // namespace Core

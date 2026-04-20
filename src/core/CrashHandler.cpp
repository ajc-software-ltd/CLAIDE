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
#include <iomanip>
#include <string>
#include <vector>

#ifdef __linux__
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>
#include <unistd.h>
#endif

#include <spdlog/spdlog.h>

namespace Core {

std::filesystem::path CrashHandler::s_crashDir;
std::filesystem::path CrashHandler::s_logDir;
volatile sig_atomic_t CrashHandler::s_inHandler = 0;

void CrashHandler::Initialize(const std::filesystem::path& logDir) {
    s_logDir = logDir;
    s_crashDir = logDir / "crashes";
    std::filesystem::create_directories(s_crashDir);

#ifdef __linux__
    struct sigaction sa{};
    sa.sa_sigaction = HandleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND | SA_SIGINFO;

    sigaction(SIGSEGV, &sa, nullptr); // Segmentation fault
    sigaction(SIGABRT, &sa, nullptr); // Abort
    sigaction(SIGFPE, &sa, nullptr);  // Floating point exception
    sigaction(SIGILL, &sa, nullptr);  // Illegal instruction
    sigaction(SIGBUS, &sa, nullptr);  // Bus error
    sigaction(SIGSYS, &sa, nullptr);  // Bad system call

    std::set_terminate(HandleTerminate);

    spdlog::info("CrashHandler: signal handlers installed");
#endif
}

#ifdef __linux__

static const char* SignalName(int signal) {
    switch (signal) {
    case SIGSEGV:
        return "SIGSEGV (Segmentation fault)";
    case SIGABRT:
        return "SIGABRT (Abort)";
    case SIGFPE:
        return "SIGFPE (Floating point exception)";
    case SIGILL:
        return "SIGILL (Illegal instruction)";
    case SIGBUS:
        return "SIGBUS (Bus error)";
    case SIGSYS:
        return "SIGSYS (Bad system call)";
    default:
        return "Unknown signal";
    }
}

static std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&timeT);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
    return buf;
}

static void WriteStackTrace(std::ofstream& dump) {
    void* buffer[256];
    int nptrs = backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));
    char** symbols = backtrace_symbols(buffer, nptrs);

    dump << "=== Stack Trace (" << nptrs << " frames) ===" << std::endl;
    for (int i = 0; i < nptrs; ++i) {
        std::string rawSymbol(symbols[i]);

        // Format: module(mangledSymbol+offset) [address]
        auto start = rawSymbol.find('(');
        auto end = rawSymbol.find('+', start != std::string::npos ? start : 0);

        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string mangled = rawSymbol.substr(start + 1, end - start - 1);
            int status = 0;
            char* demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
            if (status == 0 && demangled) {
                dump << "  #" << std::setw(3) << i << " " << rawSymbol.substr(0, start + 1) << demangled
                     << rawSymbol.substr(end) << std::endl;
                free(demangled);
            } else {
                dump << "  #" << std::setw(3) << i << " " << symbols[i] << std::endl;
            }
        } else {
            dump << "  #" << std::setw(3) << i << " " << symbols[i] << std::endl;
        }
    }

    if (symbols)
        std::free(static_cast<void*>(symbols));
}

void CrashHandler::WriteCrashDump(int signal) {
    std::string ts = GetTimestamp();
    std::string crashFile = "crash_" + ts + ".dump";
    auto crashPath = s_crashDir / crashFile;

    std::ofstream dump(crashPath);
    if (!dump.is_open()) {
        // Fallback: write to stderr
        fprintf(stderr, "CRASH: %s — failed to write dump to %s\n", SignalName(signal), crashPath.c_str());
        return;
    }

    dump << "========================================" << std::endl;
    dump << "  CLIADE AI Content Creator — Crash Dump" << std::endl;
    dump << "========================================" << std::endl;
    dump << std::endl;
    dump << "Signal:     " << SignalName(signal) << std::endl;
    dump << "Time:       " << ts << std::endl;
    dump << "PID:        " << getpid() << std::endl;
    dump << "UID:        " << getuid() << std::endl;
    dump << "Crash dir:  " << s_crashDir.string() << std::endl;
    dump << std::endl;

    WriteStackTrace(dump);

    dump << std::endl;
    dump << "=== End Crash Dump ===" << std::endl;
    dump.flush();
    dump.close();

    // Also log to stderr (always works, even if heap is corrupted)
    fprintf(stderr, "\n");
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "  CLIADE CRASH: %s\n", SignalName(signal));
    fprintf(stderr, "  Dump written to: %s\n", crashPath.c_str());
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "\n");
    fflush(stderr);
}

void CrashHandler::HandleSignal(int signal, siginfo_t* info, void* context) {
    (void)info;
    (void)context;

    // Prevent re-entry
    if (__atomic_exchange_n(&s_inHandler, 1, __ATOMIC_SEQ_CST)) {
        _exit(128 + signal);
    }

    // Async-signal-safe emergency marker only.
    static constexpr char kSignalMessage[] =
        "CLIADE: fatal signal received; rich crash dump disabled in signal context\n";
    (void)!write(STDERR_FILENO, kSignalMessage, sizeof(kSignalMessage) - 1);

    // Generate core dump by resetting signal to default and re-raising
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

void CrashHandler::HandleTerminate() {
    if (__atomic_exchange_n(&s_inHandler, 1, __ATOMIC_SEQ_CST)) {
        _exit(1);
    }

    std::string ts = GetTimestamp();
    std::string crashFile = "terminate_" + ts + ".dump";
    auto crashPath = s_crashDir / crashFile;

    std::ofstream dump(crashPath);
    if (dump.is_open()) {
        dump << "========================================" << std::endl;
        dump << "  CLIADE — std::terminate Crash Dump" << std::endl;
        dump << "========================================" << std::endl;
        dump << std::endl;
        dump << "Cause:      Uncaught exception (std::terminate)" << std::endl;
        dump << "Time:       " << ts << std::endl;
        dump << "PID:        " << getpid() << std::endl;
        dump << std::endl;

        WriteStackTrace(dump);

        dump << std::endl;
        dump << "=== End Crash Dump ===" << std::endl;
        dump.flush();
        dump.close();

        fprintf(stderr, "\n");
        fprintf(stderr, "========================================\n");
        fprintf(stderr, "  CLIADE CRASH: std::terminate\n");
        fprintf(stderr, "  Dump written to: %s\n", crashPath.c_str());
        fprintf(stderr, "========================================\n\n");
        fflush(stderr);
    }

    // Generate core dump
    std::abort();
}

#endif

} // namespace Core

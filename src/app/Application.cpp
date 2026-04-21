// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        Application.cpp
// Project:     CLIADE
// Copyright:   \u00A9 2026 AJC-Software Ltd
// ============================================================================

#include "app/Application.hpp"

#include <filesystem>
#include <string>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <Magick++.h>
#include <wx/image.h>
#include <wx/log.h>

#include "core/CrashHandler.hpp"
#include "platform/PlatformPaths.hpp"
#include "ui/MainFrame.hpp"
#include "ui/Theme.hpp"

namespace App {

class SpdlogTarget : public wxLog
{
  public:
    void DoLogText(const wxString& message) override {
        if (message.IsEmpty())
            return;

        if (message.Lower().Find("focus") != wxNOT_FOUND) {
            return;
        }

        spdlog::info("wxWidgets: {}", message.ToUTF8().data());
    }

    void DoLogRecord(wxLogLevel level, const wxString& message, const wxLogRecordInfo& info) override {
        (void)info;

        if (message.Lower().Find("focus") != wxNOT_FOUND) {
            return;
        }

        auto text = message.ToUTF8().data();

        switch (level) {
        case wxLOG_Debug:
            spdlog::debug("wxWidgets: {}", text);
            break;
        case wxLOG_Info:
            spdlog::info("wxWidgets: {}", text);
            break;
        case wxLOG_Warning:
            spdlog::warn("wxWidgets: {}", text);
            break;
        case wxLOG_Error:
            spdlog::error("wxWidgets: {}", text);
            break;
        case wxLOG_Status:
            spdlog::info("wxWidgets: {}", text);
            break;
        default:
            spdlog::info("wxWidgets: {}", text);
            break;
        }
    }
};

#ifndef NDEBUG
void WxAssertHandler(const wxString& file, int line, const wxString& func, const wxString& cond, const wxString& msg) {
    std::string fullMsg =
        fmt::format("ASSERT FAILED: {} | file: {} | line: {} | func: {} | condition: {}", msg.ToUTF8().data(),
                    file.ToUTF8().data(), line, func.ToUTF8().data(), cond.ToUTF8().data());
    spdlog::critical(fullMsg);
}
#endif

bool Application::OnInit() {
    try {
        auto logPath = Platform::GetLogFilePath();
        auto logDir = logPath.parent_path();
        std::filesystem::create_directories(logDir);

        // Initialize crash handler
        Core::CrashHandler::Initialize(logDir);

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath.string(), 5 * 1024 * 1024, 3);

        auto logger = std::make_shared<spdlog::logger>("cliade", spdlog::sinks_init_list{consoleSink, fileSink});

#ifdef NDEBUG
        logger->set_level(spdlog::level::info);
#else
        logger->set_level(spdlog::level::debug);
#endif

        spdlog::set_default_logger(logger);
        spdlog::flush_on(spdlog::level::warn);
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize logger: {}", e.what());
    }

    spdlog::info("Application: starting CLIADE");

    wxImage::AddHandler(new wxPNGHandler());
    Magick::InitializeMagick(nullptr);

#ifndef NDEBUG
    wxHandleFatalExceptions(true);
    wxSetAssertHandler(WxAssertHandler);
#endif

    auto logTarget = new SpdlogTarget();
    wxLog::SetActiveTarget(logTarget);

    Ui::Theme::ApplyDarkTheme();

    auto frame = new Ui::MainFrame();
    frame->Show(true);
    SetTopWindow(frame);

    return true;
}

int Application::OnExit() {
    spdlog::info("Application: exiting");
    spdlog::shutdown();
    return wxApp::OnExit();
}

} // namespace App

wxIMPLEMENT_APP(App::Application);

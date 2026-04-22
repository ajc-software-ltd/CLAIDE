#include "core/ShaderCompiler.hpp"

#include <cstdlib>
#include <filesystem>
#include <string>

#include <spdlog/spdlog.h>

#include "platform/PlatformPaths.hpp"

namespace Core {

bool ShaderCompiler::CompileProjectShaders() {
    const auto projectRoot = Platform::GetProjectRoot();
    const auto shaderDir = projectRoot / "src" / "vulkan" / "shaders";
    const auto outputDir = projectRoot / "build" / "shaders";

    if (!std::filesystem::exists(shaderDir)) {
        spdlog::warn("ShaderCompiler: shader directory missing: {}", shaderDir.string());
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(outputDir, ec);
    if (ec) {
        spdlog::warn("ShaderCompiler: failed to create shader output directory: {}", ec.message());
        return false;
    }

    bool compiledAny = false;
    bool allOk = true;
    for (const auto& entry : std::filesystem::directory_iterator(shaderDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto extension = entry.path().extension().string();
        if (extension != ".comp" && extension != ".vert" && extension != ".frag") {
            continue;
        }

        compiledAny = true;
        const auto outputPath = outputDir / (entry.path().stem().string() + ".spv");
        const std::string command =
            "glslangValidator -V \"" + entry.path().string() + "\" -o \"" + outputPath.string() + "\"";
        const int code = std::system(command.c_str());
        if (code != 0) {
            allOk = false;
            spdlog::warn("ShaderCompiler: glslangValidator failed for {}", entry.path().string());
        }
    }

    if (!compiledAny) {
        spdlog::warn("ShaderCompiler: no shader sources found in {}", shaderDir.string());
        return false;
    }

    if (allOk) {
        spdlog::info("ShaderCompiler: shader compilation complete at startup");
    }

    return allOk;
}

} // namespace Core

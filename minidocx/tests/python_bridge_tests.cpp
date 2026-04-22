#include "minidocx/providers/python_bridge.hpp"

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace
{
void require(const bool condition, const std::string& message)
{
  if (!condition)
    throw std::runtime_error(message);
}

bool hasPython()
{
#if defined(_WIN32)
  return std::system("python --version > NUL 2>&1") == 0 || std::system("python3 --version > NUL 2>&1") == 0;
#else
  return std::system("python3 --version > /dev/null 2>&1") == 0 || std::system("python --version > /dev/null 2>&1") == 0;
#endif
}
}

int main()
{
  using namespace md::providers;

  PythonBridgeConfig disabled;
  disabled.enabled = false;
  const auto disabledResponse = probePythonProviders(disabled);
  require(disabledResponse.code == PythonBridgeCode::Disabled, "disabled bridge should return Disabled code");

  if (!hasPython())
    return 0;

  PythonBridgeConfig cfg;
  cfg.enabled = true;
  cfg.pythonExecutable = "python3";
  cfg.workerScript = std::string(MINIDOCX_SOURCE_DIR) + "/python/worker.py";

  require(std::filesystem::exists(cfg.workerScript), "python worker script should exist");

  const auto probe = probePythonProviders(cfg);
  require(probe.code == PythonBridgeCode::Ok, "provider probe should succeed when python worker is available");

  PythonProviderRequest smokeRequest;
  smokeRequest.provider = "smoke";
  smokeRequest.operation = "ping";
  const auto smoke = invokePythonProvider(cfg, smokeRequest);
  require(smoke.code == PythonBridgeCode::Ok, "smoke provider ping should succeed");
  require(smoke.text == "pong", "smoke provider should respond with pong");

  PythonProviderRequest mammothRequest;
  mammothRequest.provider = "mammoth";
  mammothRequest.operation = "docx_to_html";
  mammothRequest.inputPath = "missing.docx";
  const auto mammoth = invokePythonProvider(cfg, mammothRequest);
  require(mammoth.code == PythonBridgeCode::ProviderUnavailable || mammoth.code == PythonBridgeCode::InvalidRequest,
          "mammoth unavailable/missing-input path should be deterministic");

  return 0;
}

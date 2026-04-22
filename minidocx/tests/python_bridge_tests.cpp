#include "minidocx/providers/python_bridge.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
void require(const bool condition, const std::string& message)
{
  if (!condition)
    throw std::runtime_error(message);
}

std::string findPythonExecutable()
{
#if defined(_WIN32)
  if (std::system("python --version > NUL 2>&1") == 0)
    return "python";
  if (std::system("python3 --version > NUL 2>&1") == 0)
    return "python3";
#else
  if (std::system("python3 --version > /dev/null 2>&1") == 0)
    return "python3";
  if (std::system("python --version > /dev/null 2>&1") == 0)
    return "python";
#endif
  return {};
}

#ifndef _WIN32
std::filesystem::path writeFakeWorker(const std::string& name, const std::string& body)
{
  const auto scriptPath = std::filesystem::temp_directory_path() / name;
  std::ofstream os(scriptPath);
  os << "#!/usr/bin/env bash\n";
  os << "req=\"\"\nresp=\"\"\n";
  os << "while [[ $# -gt 0 ]]; do\n";
  os << "  case \"$1\" in\n";
  os << "    --request) req=\"$2\"; shift 2 ;;\n";
  os << "    --response) resp=\"$2\"; shift 2 ;;\n";
  os << "    *) shift ;;\n";
  os << "  esac\n";
  os << "done\n";
  os << body << "\n";
  os.close();
  std::filesystem::permissions(
      scriptPath,
      std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
      std::filesystem::perm_options::replace);
  return scriptPath;
}
#endif
}

int main()
{
  using namespace md::providers;

  PythonBridgeConfig disabled;
  disabled.enabled = false;
  const auto disabledResponse = probePythonProviders(disabled);
  require(disabledResponse.code == PythonBridgeCode::Disabled, "disabled bridge should return Disabled code");
  require(disabledResponse.provenance == PythonResultProvenance::Native, "disabled result should be native provenance");

  PythonBridgeConfig missingPaths;
  missingPaths.enabled = true;
  const auto unavailable = probePythonProviders(missingPaths);
  require(unavailable.code == PythonBridgeCode::BridgeUnavailable, "missing explicit paths should produce bridge unavailable");

  const std::string pythonExe = findPythonExecutable();
  if (pythonExe.empty())
    return 0;

  PythonBridgeConfig cfg;
  cfg.enabled = true;
  cfg.pythonExecutablePath = pythonExe;
  cfg.workerScriptPath = std::string(MINIDOCX_SOURCE_DIR) + "/python/worker.py";

  require(std::filesystem::exists(cfg.workerScriptPath), "python worker script should exist");

  const auto probe = probePythonProviders(cfg);
  require(probe.code == PythonBridgeCode::Ok, "provider probe should succeed when python worker is available");
  require(!probe.providers.empty(), "probe should return provider details");

  PythonProviderRequest smokeRequest;
  smokeRequest.provider = "smoke";
  smokeRequest.operation = "ping";
  const auto smoke = invokePythonProvider(cfg, smokeRequest);
  require(smoke.code == PythonBridgeCode::Ok, "smoke provider ping should succeed");
  require(smoke.text == "pong", "smoke provider should respond with pong");
  require(smoke.provenance == PythonResultProvenance::PythonProvider, "smoke should be python provenance");

  PythonProviderRequest mammothRequest;
  mammothRequest.provider = "mammoth";
  mammothRequest.operation = "docx_to_html";
  mammothRequest.inputPath = "missing.docx";
  const auto mammoth = invokePythonProvider(cfg, mammothRequest);
  require(mammoth.code == PythonBridgeCode::ProviderUnavailable || mammoth.code == PythonBridgeCode::InvalidRequest,
          "mammoth unavailable/missing-input path should be deterministic");

#ifndef _WIN32
  {
    PythonBridgeConfig fakeCfg;
    fakeCfg.enabled = true;
    fakeCfg.workerExecutablePath = writeFakeWorker(
        "minidocx_fake_bad_resp.sh", "echo 'code=ok' > \"$resp\"");
    const auto malformed = probePythonProviders(fakeCfg);
    require(malformed.code == PythonBridgeCode::MalformedResponse,
            "missing protocol_version in fake response should be malformed");
    std::filesystem::remove(fakeCfg.workerExecutablePath);
  }

  {
    PythonBridgeConfig fakeCfg;
    fakeCfg.enabled = true;
    fakeCfg.workerExecutablePath = writeFakeWorker(
        "minidocx_fake_proto_mismatch.sh", "echo 'protocol_version=999' > \"$resp\"; echo 'code=ok' >> \"$resp\"");
    const auto mismatch = probePythonProviders(fakeCfg);
    require(mismatch.code == PythonBridgeCode::ProtocolMismatch,
            "protocol mismatch must be normalized");
    std::filesystem::remove(fakeCfg.workerExecutablePath);
  }
#endif

  return 0;
}

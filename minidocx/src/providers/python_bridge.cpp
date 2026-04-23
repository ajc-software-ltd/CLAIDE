#include "providers/python_bridge.hpp"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace MINIDOCX_NAMESPACE::providers
{
namespace
{
struct LaunchPlan
{
  bool valid = false;
  std::string mode;
  std::string executable;
  std::string script;
  std::string command;
  std::string error;
};

std::string trim(std::string value)
{
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
    value.erase(value.begin());
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
    value.pop_back();
  return value;
}

std::string escapeField(std::string value)
{
  size_t pos = 0;
  while ((pos = value.find('\n', pos)) != std::string::npos) {
    value.replace(pos, 1, "\\n");
    pos += 2;
  }
  pos = 0;
  while ((pos = value.find('|', pos)) != std::string::npos) {
    value.replace(pos, 1, "\\p");
    pos += 2;
  }
  pos = 0;
  while ((pos = value.find(';', pos)) != std::string::npos) {
    value.replace(pos, 1, "\\s");
    pos += 2;
  }
  pos = 0;
  while ((pos = value.find('\\', pos)) != std::string::npos) {
    value.replace(pos, 1, "\\\\");
    pos += 2;
  }
  return value;
}

std::string unescapeField(std::string value)
{
  std::string out;
  out.reserve(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '\\' && i + 1 < value.size()) {
      const char c = value[i + 1];
      if (c == 'n') {
        out.push_back('\n');
        ++i;
        continue;
      }
      if (c == 'p') {
        out.push_back('|');
        ++i;
        continue;
      }
      if (c == 's') {
        out.push_back(';');
        ++i;
        continue;
      }
      if (c == '\\') {
        out.push_back('\\');
        ++i;
        continue;
      }
    }
    out.push_back(value[i]);
  }
  return out;
}

const char k_base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64Encode(const std::string& input)
{
  std::string out;
  int val = 0;
  int valb = -6;
  for (const unsigned char c : input) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(k_base64Table[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(k_base64Table[((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4)
    out.push_back('=');
  return out;
}

std::string base64Decode(const std::string& input)
{
  std::vector<int> table(256, -1);
  for (int i = 0; i < 64; i++)
    table[static_cast<unsigned char>(k_base64Table[i])] = i;

  std::string out;
  int val = 0;
  int valb = -8;
  for (const unsigned char c : input) {
    if (table[c] == -1)
      break;
    val = (val << 6) + table[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

bool writeRequestFile(const std::filesystem::path& path, const PythonProviderRequest& request)
{
  std::ofstream os(path, std::ios::binary);
  if (!os)
    return false;

  os << "protocol_version=" << k_pythonBridgeProtocolVersion << '\n';
  os << "provider=" << escapeField(request.provider) << '\n';
  os << "operation=" << escapeField(request.operation) << '\n';
  os << "input_path=" << escapeField(request.inputPath) << '\n';
  os << "output_path=" << escapeField(request.outputPath) << '\n';
  os << "payload_b64=" << base64Encode(request.payload) << '\n';
  return true;
}

std::map<std::string, std::string> readKeyValues(const std::filesystem::path& path)
{
  std::map<std::string, std::string> out;
  std::ifstream is(path, std::ios::binary);
  if (!is)
    return out;

  std::string line;
  while (std::getline(is, line)) {
    const auto pos = line.find('=');
    if (pos == std::string::npos)
      continue;
    out[trim(line.substr(0, pos))] = line.substr(pos + 1);
  }
  return out;
}


std::vector<std::string> splitLines(const std::string& text)
{
  std::vector<std::string> out;
  std::stringstream ss(text);
  std::string line;
  while (std::getline(ss, line)) {
    if (!line.empty())
      out.push_back(line);
  }
  return out;
}

std::vector<std::string> split(const std::string& in, const char sep)
{
  std::vector<std::string> out;
  std::stringstream ss(in);
  std::string item;
  while (std::getline(ss, item, sep))
    out.push_back(item);
  return out;
}

std::vector<ProviderInfo> parseProviders(const std::string& packed)
{
  std::vector<ProviderInfo> providers;
  if (packed.empty())
    return providers;

  for (const auto& entry : split(packed, ';')) {
    if (entry.empty())
      continue;
    auto fields = split(entry, '|');
    if (fields.size() < 5)
      continue;

    ProviderInfo info;
    info.name = unescapeField(fields[0]);
    info.available = fields[1] == "1";
    info.version = unescapeField(fields[2]);
    for (const auto& cap : split(unescapeField(fields[3]), ',')) {
      if (!cap.empty())
        info.capabilities.push_back(cap);
    }
    info.message = unescapeField(fields[4]);
    providers.push_back(std::move(info));
  }

  return providers;
}

PythonBridgeCode parseCode(const std::string& code)
{
  if (code == "ok")
    return PythonBridgeCode::Ok;
  if (code == "bridge_unavailable")
    return PythonBridgeCode::BridgeUnavailable;
  if (code == "worker_launch_failed")
    return PythonBridgeCode::WorkerLaunchFailed;
  if (code == "provider_unavailable")
    return PythonBridgeCode::ProviderUnavailable;
  if (code == "provider_version_unsupported")
    return PythonBridgeCode::ProviderVersionUnsupported;
  if (code == "invalid_request")
    return PythonBridgeCode::InvalidRequest;
  if (code == "malformed_response")
    return PythonBridgeCode::MalformedResponse;
  if (code == "protocol_mismatch")
    return PythonBridgeCode::ProtocolMismatch;
  if (code == "timed_out")
    return PythonBridgeCode::TimedOut;
  if (code == "cancelled")
    return PythonBridgeCode::Cancelled;
  if (code == "execution_failed")
    return PythonBridgeCode::ExecutionFailed;
  return PythonBridgeCode::MalformedResponse;
}

PythonResultProvenance parseProvenance(const std::string& provenance)
{
  if (provenance == "native")
    return PythonResultProvenance::Native;
  if (provenance == "mixed")
    return PythonResultProvenance::MixedAssisted;
  return PythonResultProvenance::PythonProvider;
}

LaunchPlan resolveWorkerLaunch(const PythonBridgeConfig& config)
{
  LaunchPlan plan;
  if (!config.workerExecutablePath.empty()) {
    plan.valid = true;
    plan.mode = "worker_executable";
    plan.executable = config.workerExecutablePath;
    return plan;
  }

  if (!config.pythonExecutablePath.empty() && !config.workerScriptPath.empty()) {
    plan.valid = true;
    plan.mode = "python_script";
    plan.executable = config.pythonExecutablePath;
    plan.script = config.workerScriptPath;
    return plan;
  }

  plan.error =
      "bridge unavailable: configure either workerExecutablePath or pythonExecutablePath + workerScriptPath";
  return plan;
}

PythonProviderResponse callWorker(const PythonBridgeConfig& config, const PythonProviderRequest& request)
{
  PythonProviderResponse response;
  response.providerName = request.provider;
  response.providerOperation = request.operation;
  response.provenance = PythonResultProvenance::PythonProvider;

  const LaunchPlan plan = resolveWorkerLaunch(config);
  response.launchMode = plan.mode;
  response.launchTarget = !plan.executable.empty() ? plan.executable : plan.script;

  if (!plan.valid) {
    response.code = PythonBridgeCode::BridgeUnavailable;
    response.message = plan.error;
    return response;
  }

  const auto token = std::to_string(std::rand());
  const auto tempDir = std::filesystem::temp_directory_path();
  const auto reqPath = tempDir / ("minidocx_py_req_" + token + ".txt");
  const auto respPath = tempDir / ("minidocx_py_resp_" + token + ".txt");

  if (!writeRequestFile(reqPath, request)) {
    response.code = PythonBridgeCode::InvalidRequest;
    response.message = "failed to write worker request file";
    return response;
  }

  if (plan.mode == "worker_executable") {
    response.launchTarget = plan.executable;
    response.launchMode = plan.mode;
    response.debugDetail = "mode=worker_executable";
  } else {
    response.launchTarget = plan.executable + " " + plan.script;
    response.launchMode = plan.mode;
    response.debugDetail = "mode=python_script";
  }

  const std::string cmd = plan.mode == "worker_executable"
      ? ("\"" + plan.executable + "\" --request \"" + reqPath.string() + "\" --response \"" + respPath.string() + "\"")
      : ("\"" + plan.executable + "\" \"" + plan.script + "\" --request \"" + reqPath.string() + "\" --response \"" +
          respPath.string() + "\"");

  const int rc = std::system(cmd.c_str());
  if (rc != 0) {
    std::filesystem::remove(reqPath);
    std::filesystem::remove(respPath);
    response.code = PythonBridgeCode::WorkerLaunchFailed;
    response.message = "python worker launch failed";
    return response;
  }

  const auto values = readKeyValues(respPath);
  std::filesystem::remove(reqPath);
  std::filesystem::remove(respPath);

  if (values.empty()) {
    response.code = PythonBridgeCode::MalformedResponse;
    response.message = "empty worker response";
    return response;
  }

  if (const auto itVersion = values.find("protocol_version"); itVersion == values.end()) {
    response.code = PythonBridgeCode::MalformedResponse;
    response.message = "missing protocol_version in response";
    return response;
  } else if (itVersion->second != std::to_string(k_pythonBridgeProtocolVersion)) {
    response.code = PythonBridgeCode::ProtocolMismatch;
    response.message = "protocol version mismatch";
    return response;
  }

  if (const auto itCode = values.find("code"); itCode == values.end()) {
    response.code = PythonBridgeCode::MalformedResponse;
    response.message = "missing response code";
    return response;
  } else {
    response.code = parseCode(itCode->second);
  }

  if (const auto itMessage = values.find("message"); itMessage != values.end())
    response.message = unescapeField(itMessage->second);

  if (const auto itDebug = values.find("debug_detail"); itDebug != values.end())
    response.debugDetail = unescapeField(itDebug->second);

  if (const auto itText = values.find("text_b64"); itText != values.end())
    response.text = base64Decode(itText->second);

  if (const auto itOutPath = values.find("output_path"); itOutPath != values.end())
    response.outputPath = unescapeField(itOutPath->second);

  if (const auto itWarnings = values.find("warnings_b64"); itWarnings != values.end())
    response.warnings = splitLines(base64Decode(itWarnings->second));

  if (const auto itProviderVersion = values.find("provider_version"); itProviderVersion != values.end())
    response.providerVersion = unescapeField(itProviderVersion->second);

  if (const auto itProv = values.find("provenance"); itProv != values.end())
    response.provenance = parseProvenance(itProv->second);

  if (const auto itProviders = values.find("providers"); itProviders != values.end())
    response.providers = parseProviders(itProviders->second);

  return response;
}
} // namespace

PythonProviderResponse probePythonProviders(const PythonBridgeConfig& config)
{
  if (!config.enabled) {
    PythonProviderResponse response;
    response.code = PythonBridgeCode::Disabled;
    response.message = "python bridge disabled by configuration";
    response.provenance = PythonResultProvenance::Native;
    return response;
  }

  PythonProviderRequest request;
  request.provider = "system";
  request.operation = "probe";
  return callWorker(config, request);
}

PythonProviderResponse invokePythonProvider(
    const PythonBridgeConfig& config,
    const PythonProviderRequest& request)
{
  if (!config.enabled) {
    PythonProviderResponse response;
    response.code = PythonBridgeCode::Disabled;
    response.message = "python bridge disabled by configuration";
    response.provenance = PythonResultProvenance::Native;
    return response;
  }

  if (request.provider.empty() || request.operation.empty()) {
    PythonProviderResponse response;
    response.code = PythonBridgeCode::InvalidRequest;
    response.message = "provider and operation are required";
    response.provenance = PythonResultProvenance::PythonProvider;
    response.providerName = request.provider;
    response.providerOperation = request.operation;
    return response;
  }

  return callWorker(config, request);
}
} // namespace MINIDOCX_NAMESPACE::providers

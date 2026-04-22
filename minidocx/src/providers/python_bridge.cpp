#include "providers/python_bridge.hpp"

#include <cctype>
#include <cstdio>
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
  while ((pos = value.find('\\', pos)) != std::string::npos) {
    value.replace(pos, 1, "\\\\");
    pos += 2;
  }
  pos = 0;
  while ((pos = value.find('\n', pos)) != std::string::npos) {
    value.replace(pos, 1, "\\n");
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
  for (unsigned char c : input) {
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
    const std::string key = trim(line.substr(0, pos));
    const std::string value = line.substr(pos + 1);
    out[key] = value;
  }
  return out;
}

PythonBridgeCode parseCode(const std::string& code)
{
  if (code == "ok")
    return PythonBridgeCode::Ok;
  if (code == "disabled")
    return PythonBridgeCode::Disabled;
  if (code == "worker_unavailable")
    return PythonBridgeCode::WorkerUnavailable;
  if (code == "invalid_request")
    return PythonBridgeCode::InvalidRequest;
  if (code == "provider_unavailable")
    return PythonBridgeCode::ProviderUnavailable;
  if (code == "operation_failed")
    return PythonBridgeCode::OperationFailed;
  return PythonBridgeCode::InvalidResponse;
}

std::vector<ProviderInfo> parseProviders(const std::string& packed)
{
  std::vector<ProviderInfo> providers;
  if (packed.empty())
    return providers;

  std::stringstream ss(packed);
  std::string item;
  while (std::getline(ss, item, ';')) {
    if (item.empty())
      continue;
    std::stringstream p(item);
    std::string field;
    std::vector<std::string> fields;
    while (std::getline(p, field, '|'))
      fields.push_back(unescapeField(field));
    if (fields.size() < 4)
      continue;
    ProviderInfo info;
    info.name = fields[0];
    info.available = (fields[1] == "1");
    info.version = fields[2];
    info.message = fields[3];
    providers.push_back(std::move(info));
  }
  return providers;
}

PythonProviderResponse callWorker(const PythonBridgeConfig& config, const PythonProviderRequest& request)
{
  PythonProviderResponse response;

  const auto tempDir = std::filesystem::temp_directory_path();
  const auto token = std::to_string(std::rand());
  const auto reqPath = tempDir / ("minidocx_py_req_" + token + ".txt");
  const auto respPath = tempDir / ("minidocx_py_resp_" + token + ".txt");

  if (!writeRequestFile(reqPath, request)) {
    response.code = PythonBridgeCode::InvalidRequest;
    response.message = "failed to write worker request file";
    return response;
  }

  const std::string cmd = "\"" + config.pythonExecutable + "\" \"" + config.workerScript +
      "\" --request \"" + reqPath.string() + "\" --response \"" + respPath.string() + "\"";

  const int rc = std::system(cmd.c_str());
  if (rc != 0) {
    std::filesystem::remove(reqPath);
    std::filesystem::remove(respPath);
    response.code = PythonBridgeCode::WorkerExecutionFailed;
    response.message = "python worker execution failed";
    return response;
  }

  const auto values = readKeyValues(respPath);
  std::filesystem::remove(reqPath);
  std::filesystem::remove(respPath);

  if (values.empty()) {
    response.code = PythonBridgeCode::InvalidResponse;
    response.message = "empty python worker response";
    return response;
  }

  const auto itCode = values.find("code");
  if (itCode == values.end()) {
    response.code = PythonBridgeCode::InvalidResponse;
    response.message = "missing response code";
    return response;
  }

  response.code = parseCode(itCode->second);

  if (const auto itMessage = values.find("message"); itMessage != values.end())
    response.message = unescapeField(itMessage->second);

  if (const auto itText = values.find("text_b64"); itText != values.end())
    response.text = base64Decode(itText->second);

  if (const auto itOutPath = values.find("output_path"); itOutPath != values.end())
    response.outputPath = unescapeField(itOutPath->second);

  if (const auto itProviders = values.find("providers"); itProviders != values.end())
    response.providers = parseProviders(itProviders->second);

  return response;
}
}

PythonProviderResponse probePythonProviders(const PythonBridgeConfig& config)
{
  if (!config.enabled)
    return {PythonBridgeCode::Disabled, "python bridge disabled by configuration", {}, {}, {}};

  PythonProviderRequest request;
  request.provider = "system";
  request.operation = "probe";
  return callWorker(config, request);
}

PythonProviderResponse invokePythonProvider(
    const PythonBridgeConfig& config,
    const PythonProviderRequest& request)
{
  if (!config.enabled)
    return {PythonBridgeCode::Disabled, "python bridge disabled by configuration", {}, {}, {}};

  if (request.provider.empty() || request.operation.empty())
    return {PythonBridgeCode::InvalidRequest, "provider and operation are required", {}, {}, {}};

  return callWorker(config, request);
}
}

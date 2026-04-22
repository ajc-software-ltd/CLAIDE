#pragma once

#include "config.hpp"

#include <string>
#include <vector>

namespace MINIDOCX_NAMESPACE::providers
{
  enum class PythonBridgeCode
  {
    Ok,
    Disabled,
    WorkerUnavailable,
    WorkerExecutionFailed,
    InvalidRequest,
    InvalidResponse,
    ProviderUnavailable,
    OperationFailed,
  };

  struct ProviderInfo
  {
    std::string name;
    bool available = false;
    std::string version;
    std::string message;
  };

  struct PythonBridgeConfig
  {
    bool enabled = false;
    std::string pythonExecutable = "python3";
    std::string workerScript = "python/worker.py";
  };

  struct PythonProviderRequest
  {
    std::string provider;
    std::string operation;
    std::string inputPath;
    std::string outputPath;
    std::string payload;
  };

  struct PythonProviderResponse
  {
    PythonBridgeCode code = PythonBridgeCode::Ok;
    std::string message;
    std::string text;
    std::string outputPath;
    std::vector<ProviderInfo> providers;
  };

  MINIDOCX_API PythonProviderResponse probePythonProviders(const PythonBridgeConfig& config);

  MINIDOCX_API PythonProviderResponse invokePythonProvider(
      const PythonBridgeConfig& config,
      const PythonProviderRequest& request);
}

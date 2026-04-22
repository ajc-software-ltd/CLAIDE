#pragma once

#include "config.hpp"

#include <string>
#include <vector>

namespace MINIDOCX_NAMESPACE::providers
{
  inline constexpr int k_pythonBridgeProtocolVersion = 1;

  enum class PythonBridgeCode
  {
    Ok,
    Disabled,
    BridgeUnavailable,
    WorkerLaunchFailed,
    ProviderUnavailable,
    ProviderVersionUnsupported,
    InvalidRequest,
    MalformedResponse,
    ProtocolMismatch,
    ExecutionFailed,
    TimedOut,
    Cancelled,
  };

  enum class PythonResultProvenance
  {
    Native,
    PythonProvider,
    MixedAssisted,
  };

  struct ProviderInfo
  {
    std::string name;
    bool available = false;
    std::string version;
    std::vector<std::string> capabilities;
    std::string message;
  };

  struct PythonBridgeConfig
  {
    bool enabled = false;

    // Explicit mode A: direct worker executable path.
    std::string workerExecutablePath;

    // Explicit mode B: python executable + worker script path.
    std::string pythonExecutablePath;
    std::string workerScriptPath;

    int timeoutMs = 30000;
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
    std::string debugDetail;

    std::string text;
    std::string outputPath;

    PythonResultProvenance provenance = PythonResultProvenance::PythonProvider;
    std::string providerName;
    std::string providerOperation;

    std::string launchMode;
    std::string launchTarget;

    std::vector<ProviderInfo> providers;
  };

  MINIDOCX_API PythonProviderResponse probePythonProviders(const PythonBridgeConfig& config);

  MINIDOCX_API PythonProviderResponse invokePythonProvider(
      const PythonBridgeConfig& config,
      const PythonProviderRequest& request);
}

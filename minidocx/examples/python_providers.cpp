#include "minidocx/minidocx.hpp"

#include <iostream>

int main()
{
  using namespace md;

  providers::PythonBridgeConfig cfg;
  cfg.enabled = true;
  cfg.pythonExecutablePath = "python3";
  cfg.workerScriptPath = "python/worker.py";

  const auto probe = providers::probePythonProviders(cfg);
  std::cout << "Probe code: " << static_cast<int>(probe.code) << '\n';

  providers::PythonProviderRequest renderReq;
  renderReq.provider = "docxtpl";
  renderReq.operation = "render_template";
  renderReq.inputPath = "template.docx";
  renderReq.outputPath = "rendered.docx";
  renderReq.payload = "{\"name\":\"Alice\"}";

  const auto render = providers::invokePythonProvider(cfg, renderReq);
  std::cout << "Render code: " << static_cast<int>(render.code) << '\n';

  providers::PythonProviderRequest auditReq;
  auditReq.provider = "python_docx";
  auditReq.operation = "style_audit";
  auditReq.inputPath = "rendered.docx";

  const auto audit = providers::invokePythonProvider(cfg, auditReq);
  std::cout << "Audit code: " << static_cast<int>(audit.code) << '\n';

  return 0;
}

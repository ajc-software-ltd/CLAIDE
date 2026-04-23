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

  providers::PythonProviderRequest xpathReq;
  xpathReq.provider = "lxml";
  xpathReq.operation = "xpath_query";
  xpathReq.inputPath = "rendered.docx";
  xpathReq.payload =
      R"({"part":"word/document.xml","xpath":"count(//w:p)","namespaces":{"w":"http://schemas.openxmlformats.org/wordprocessingml/2006/main"},"mode":"xpath"})";
  const auto xpath = providers::invokePythonProvider(cfg, xpathReq);
  std::cout << "XPath code: " << static_cast<int>(xpath.code) << '\n';

  providers::PythonProviderRequest xsltReq;
  xsltReq.provider = "lxml";
  xsltReq.operation = "xslt_transform";
  xsltReq.inputPath = "rendered.docx";
  xsltReq.payload =
      R"({"part":"word/document.xml","output_mode":"text","params":{"prefix":"p-count:"},"xslt":"<?xml version=\"1.0\" encoding=\"UTF-8\"?><xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><xsl:param name=\"prefix\"/><xsl:output method=\"text\"/><xsl:template match=\"/\"><xsl:value-of select=\"$prefix\"/><xsl:value-of select=\"count(//w:p)\"/></xsl:template></xsl:stylesheet>"})";
  const auto xslt = providers::invokePythonProvider(cfg, xsltReq);
  std::cout << "XSLT code: " << static_cast<int>(xslt.code) << '\n';

  return 0;
}

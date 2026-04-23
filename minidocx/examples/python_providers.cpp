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

  providers::PythonProviderRequest schematronReq;
  schematronReq.provider = "schematron";
  schematronReq.operation = "validate_part";
  schematronReq.inputPath = "rendered.docx";
  schematronReq.payload =
      R"({"part":"word/document.xml","store_report":true,"schema_text":"<sch:schema xmlns:sch=\"http://purl.oclc.org/dsdl/schematron\" queryBinding=\"xslt\"><sch:ns prefix=\"w\" uri=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/><sch:pattern id=\"p\"><sch:rule context=\"w:document\"><sch:assert test=\"count(//w:p) &gt;= 1\">document should have at least one paragraph</sch:assert></sch:rule></sch:pattern></sch:schema>"})";
  const auto schematron = providers::invokePythonProvider(cfg, schematronReq);
  std::cout << "Schematron code: " << static_cast<int>(schematron.code) << '\n';

  providers::PythonProviderRequest ocrReq;
  ocrReq.provider = "ocr";
  ocrReq.operation = "extract_text";
  ocrReq.payload =
      R"({"image_b64":"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO7Zf6sAAAAASUVORK5CYII=","image_format":"png","lang":"eng","psm":6})";
  const auto ocr = providers::invokePythonProvider(cfg, ocrReq);
  std::cout << "OCR code: " << static_cast<int>(ocr.code) << '\n';

  providers::PythonProviderRequest pdfReq;
  pdfReq.provider = "pypdf";
  pdfReq.operation = "extract_text";
  pdfReq.inputPath = "sample.pdf";
  pdfReq.payload = R"({"mode":"plain"})";
  const auto pdf = providers::invokePythonProvider(cfg, pdfReq);
  std::cout << "PDF extract code: " << static_cast<int>(pdf.code) << '\n';

  return 0;
}

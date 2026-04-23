#include "minidocx/minidocx.hpp"

#include <iostream>

int main()
{
  using namespace md;

  providers::PythonBridgeConfig cfg;
  cfg.enabled = true;
  cfg.pythonExecutablePath = "python3";
  cfg.workerScriptPath = "python/worker.py";

  providers::PythonProviderRequest xpathReq;
  xpathReq.provider = "lxml";
  xpathReq.operation = "xpath_query";
  xpathReq.inputPath = "rendered.docx";
  xpathReq.payload =
      R"JSON({"part":"word/document.xml","xpath":"count(//w:p)","namespaces":{"w":"http://schemas.openxmlformats.org/wordprocessingml/2006/main"},"mode":"xpath"})JSON";
  std::cout << "lxml xpath code: " << static_cast<int>(providers::invokePythonProvider(cfg, xpathReq).code) << '\n';

  providers::PythonProviderRequest schematronReq;
  schematronReq.provider = "schematron";
  schematronReq.operation = "validate_part";
  schematronReq.inputPath = "rendered.docx";
  schematronReq.payload =
      R"JSON({"part":"word/document.xml","store_report":true,"schema_text":"<sch:schema xmlns:sch=\"http://purl.oclc.org/dsdl/schematron\" queryBinding=\"xslt\"><sch:ns prefix=\"w\" uri=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/><sch:pattern id=\"p\"><sch:rule context=\"w:document\"><sch:assert test=\"count(//w:p) &gt;= 1\">document should have at least one paragraph</sch:assert></sch:rule></sch:pattern></sch:schema>"})JSON";
  std::cout << "schematron code: " << static_cast<int>(providers::invokePythonProvider(cfg, schematronReq).code)
            << '\n';

  return 0;
}

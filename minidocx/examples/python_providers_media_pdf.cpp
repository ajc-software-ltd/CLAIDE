#include "minidocx/minidocx.hpp"

#include <iostream>

int main()
{
  using namespace md;

  providers::PythonBridgeConfig cfg;
  cfg.enabled = true;
  cfg.pythonExecutablePath = "python3";
  cfg.workerScriptPath = "python/worker.py";

  providers::PythonProviderRequest ocrReq;
  ocrReq.provider = "ocr";
  ocrReq.operation = "extract_text";
  ocrReq.payload =
      R"JSON({"image_b64":"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO7Zf6sAAAAASUVORK5CYII=","image_format":"png","lang":"eng","psm":6})JSON";
  std::cout << "ocr code: " << static_cast<int>(providers::invokePythonProvider(cfg, ocrReq).code) << '\n';

  providers::PythonProviderRequest pypdfReq;
  pypdfReq.provider = "pypdf";
  pypdfReq.operation = "extract_text";
  pypdfReq.inputPath = "sample.pdf";
  pypdfReq.payload = R"JSON({"mode":"plain"})JSON";
  std::cout << "pypdf code: " << static_cast<int>(providers::invokePythonProvider(cfg, pypdfReq).code) << '\n';

  providers::PythonProviderRequest pdfminerReq;
  pdfminerReq.provider = "pdfminer";
  pdfminerReq.operation = "extract_layout";
  pdfminerReq.inputPath = "sample.pdf";
  pdfminerReq.payload = R"JSON({"page_numbers":[0],"laparams":{"char_margin":2.0}})JSON";
  std::cout << "pdfminer code: " << static_cast<int>(providers::invokePythonProvider(cfg, pdfminerReq).code) << '\n';

  return 0;
}

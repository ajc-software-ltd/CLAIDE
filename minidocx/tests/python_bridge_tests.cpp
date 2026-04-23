#include "minidocx/minidocx.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <vector>

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

bool providerAvailable(
    const std::vector<md::providers::ProviderInfo>& providers,
    const std::string& name)
{
  for (const auto& p : providers) {
    if (p.name == name)
      return p.available;
  }
  return false;
}

void writeSimplePdfWithText(const std::filesystem::path& path, const std::string& text)
{
  std::ofstream os(path, std::ios::binary);
  if (!os)
    throw std::runtime_error("failed to open pdf output path");

  const std::string stream = "BT\n/F1 18 Tf\n40 120 Td\n(" + text + ") Tj\nET\n";

  std::vector<std::string> objects;
  objects.emplace_back("<< /Type /Catalog /Pages 2 0 R >>");
  objects.emplace_back("<< /Type /Pages /Kids [3 0 R] /Count 1 >>");
  objects.emplace_back("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] /Resources << /Font << /F1 5 0 R >> >> /Contents 4 0 R >>");
  objects.emplace_back("<< /Length " + std::to_string(stream.size()) + " >>\nstream\n" + stream + "endstream");
  objects.emplace_back("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");

  os << "%PDF-1.4\n";

  std::vector<long> offsets;
  offsets.push_back(0);
  for (size_t i = 0; i < objects.size(); ++i) {
    offsets.push_back(static_cast<long>(os.tellp()));
    os << (i + 1) << " 0 obj\n" << objects[i] << "\nendobj\n";
  }

  const long xrefOffset = static_cast<long>(os.tellp());
  os << "xref\n";
  os << "0 " << (objects.size() + 1) << "\n";
  os << "0000000000 65535 f \n";
  for (size_t i = 1; i < offsets.size(); ++i) {
    os << std::setw(10) << std::setfill('0') << offsets[i] << " 00000 n \n";
  }
  os << "trailer\n<< /Size " << (objects.size() + 1) << " /Root 1 0 R >>\n";
  os << "startxref\n" << xrefOffset << "\n%%EOF\n";
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
} // namespace

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

  // PR13: docxtpl template rendering provider.
  const auto tempDir = std::filesystem::temp_directory_path();
  const auto templatePath = (tempDir / "minidocx_tpl_template.docx").string();
  const auto renderedPath = (tempDir / "minidocx_tpl_rendered.docx").string();

  md::Document doc;
  auto section = doc.addSection();
  section->addParagraph()->addRichText("Hello {{ name }}");
  doc.saveAs(templatePath);

  PythonProviderRequest renderRequest;
  renderRequest.provider = "docxtpl";
  renderRequest.operation = "render_template";
  renderRequest.inputPath = templatePath;
  renderRequest.outputPath = renderedPath;
  renderRequest.payload = "{\"name\":\"Alice\"}";

  const auto renderResult = invokePythonProvider(cfg, renderRequest);
  if (providerAvailable(probe.providers, "docxtpl")) {
    require(renderResult.code == PythonBridgeCode::Ok, "docxtpl render should succeed when provider is available");
    require(std::filesystem::exists(renderedPath), "rendered docx should exist");
  } else {
    require(renderResult.code == PythonBridgeCode::ProviderUnavailable,
            "docxtpl provider should return provider-unavailable when dependency is missing");
  }

  PythonProviderRequest badRenderRequest = renderRequest;
  badRenderRequest.payload = "not-json";
  const auto badRender = invokePythonProvider(cfg, badRenderRequest);
  if (providerAvailable(probe.providers, "docxtpl")) {
    require(badRender.code == PythonBridgeCode::InvalidRequest,
            "docxtpl malformed context should normalize to invalid request");
  }

  // PR13: python-docx style audit provider.
  PythonProviderRequest styleAuditRequest;
  styleAuditRequest.provider = "python_docx";
  styleAuditRequest.operation = "style_audit";
  styleAuditRequest.inputPath = templatePath;

  const auto styleAudit = invokePythonProvider(cfg, styleAuditRequest);
  if (providerAvailable(probe.providers, "python_docx")) {
    require(styleAudit.code == PythonBridgeCode::Ok, "style audit should succeed when provider is available");
    require(styleAudit.text.find("style_count") != std::string::npos,
            "style audit response should include style_count");
  } else {
    require(styleAudit.code == PythonBridgeCode::ProviderUnavailable,
            "python-docx provider should return provider-unavailable when dependency is missing");
  }

  // PR15: lxml expert XML/XPath/XSLT provider with allowlisted part contract.
  PythonProviderRequest xpathRequest;
  xpathRequest.provider = "lxml";
  xpathRequest.operation = "xpath_query";
  xpathRequest.inputPath = templatePath;
  xpathRequest.payload =
      R"({"part":"word/document.xml","xpath":"count(//w:p)","namespaces":{"w":"http://schemas.openxmlformats.org/wordprocessingml/2006/main"},"mode":"xpath"})";

  const auto xpathResult = invokePythonProvider(cfg, xpathRequest);
  if (providerAvailable(probe.providers, "lxml")) {
    require(xpathResult.code == PythonBridgeCode::Ok, "lxml xpath query should succeed when provider is available");
    require(xpathResult.text.find("\"operation\": \"xpath_query\"") != std::string::npos,
            "lxml xpath response should contain structured operation metadata");
    require(xpathResult.text.find("\"selected_part\": \"word/document.xml\"") != std::string::npos,
            "lxml xpath response should include selected part");
  } else {
    require(xpathResult.code == PythonBridgeCode::ProviderUnavailable,
            "lxml xpath should return provider-unavailable when dependency is missing");
  }

  PythonProviderRequest stylesXPath = xpathRequest;
  stylesXPath.payload =
      R"({"part":"word/styles.xml","xpath":"count(//w:style)","namespaces":{"w":"http://schemas.openxmlformats.org/wordprocessingml/2006/main"},"mode":"compiled_xpath"})";
  const auto stylesXPathResult = invokePythonProvider(cfg, stylesXPath);
  if (providerAvailable(probe.providers, "lxml")) {
    require(stylesXPathResult.code == PythonBridgeCode::Ok, "lxml styles xpath should succeed");
  }

  PythonProviderRequest numberingXPath = xpathRequest;
  numberingXPath.payload =
      R"({"part":"word/numbering.xml","xpath":"count(//w:num)","namespaces":{"w":"http://schemas.openxmlformats.org/wordprocessingml/2006/main"},"mode":"evaluator"})";
  const auto numberingXPathResult = invokePythonProvider(cfg, numberingXPath);
  if (providerAvailable(probe.providers, "lxml")) {
    require(numberingXPathResult.code == PythonBridgeCode::Ok, "lxml numbering xpath should succeed");
  }

  PythonProviderRequest disallowedPart = xpathRequest;
  disallowedPart.payload = R"({"part":"word/comments.xml","xpath":"//*"})";
  const auto disallowedPartResult = invokePythonProvider(cfg, disallowedPart);
  if (providerAvailable(probe.providers, "lxml")) {
    require(disallowedPartResult.code == PythonBridgeCode::InvalidRequest,
            "disallowed XML part should normalize to invalid request");
  }

  PythonProviderRequest invalidXPath = xpathRequest;
  invalidXPath.payload =
      R"({"part":"word/document.xml","xpath":"//*["})";
  const auto invalidXPathResult = invokePythonProvider(cfg, invalidXPath);
  if (providerAvailable(probe.providers, "lxml")) {
    require(invalidXPathResult.code == PythonBridgeCode::InvalidRequest,
            "invalid xpath should normalize to invalid request");
  }

  PythonProviderRequest xsltRequest;
  xsltRequest.provider = "lxml";
  xsltRequest.operation = "xslt_transform";
  xsltRequest.inputPath = templatePath;
  xsltRequest.payload =
      R"({"part":"word/document.xml","output_mode":"text","params":{"prefix":"P:"},"xslt":"<?xml version=\"1.0\" encoding=\"UTF-8\"?><xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><xsl:param name=\"prefix\"/><xsl:output method=\"text\"/><xsl:template match=\"/\"><xsl:value-of select=\"$prefix\"/><xsl:value-of select=\"count(//w:p)\"/></xsl:template></xsl:stylesheet>"})";
  const auto xsltResult = invokePythonProvider(cfg, xsltRequest);
  if (providerAvailable(probe.providers, "lxml")) {
    require(xsltResult.code == PythonBridgeCode::Ok, "lxml xslt transform should succeed");
    require(xsltResult.text.find("\"operation\": \"xslt_transform\"") != std::string::npos,
            "lxml xslt response should contain operation metadata");
  } else {
    require(xsltResult.code == PythonBridgeCode::ProviderUnavailable,
            "lxml xslt should return provider-unavailable when dependency is missing");
  }

  PythonProviderRequest invalidXslt = xsltRequest;
  invalidXslt.payload = R"({"part":"word/document.xml","xslt":"<xsl:stylesheet>","output_mode":"xml"})";
  const auto invalidXsltResult = invokePythonProvider(cfg, invalidXslt);
  if (providerAvailable(probe.providers, "lxml")) {
    require(invalidXsltResult.code == PythonBridgeCode::InvalidRequest,
            "invalid xslt should normalize to invalid request");
  }

  // PR17: Schematron validation provider for allowlisted DOCX XML parts.
  PythonProviderRequest schematronPass;
  schematronPass.provider = "schematron";
  schematronPass.operation = "validate_part";
  schematronPass.inputPath = templatePath;
  schematronPass.payload =
      R"({"part":"word/document.xml","store_report":true,"schema_text":"<sch:schema xmlns:sch=\"http://purl.oclc.org/dsdl/schematron\" queryBinding=\"xslt\"><sch:ns prefix=\"w\" uri=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/><sch:pattern id=\"p1\"><sch:rule context=\"w:document\"><sch:assert test=\"count(//w:p) &gt;= 1\">document should have at least one paragraph</sch:assert></sch:rule></sch:pattern></sch:schema>"})";
  const auto schematronPassResult = invokePythonProvider(cfg, schematronPass);
  if (providerAvailable(probe.providers, "schematron")) {
    require(schematronPassResult.code == PythonBridgeCode::Ok, "schematron pass case should succeed");
    require(schematronPassResult.text.find("\"operation\": \"validate_part\"") != std::string::npos,
            "schematron result should include operation metadata");
    require(schematronPassResult.text.find("\"valid\": true") != std::string::npos,
            "schematron pass case should report valid true");
  } else {
    require(schematronPassResult.code == PythonBridgeCode::ProviderUnavailable,
            "schematron should return provider-unavailable when dependency is missing");
  }

  PythonProviderRequest schematronFail = schematronPass;
  schematronFail.payload =
      R"({"part":"word/document.xml","store_report":true,"schema_text":"<sch:schema xmlns:sch=\"http://purl.oclc.org/dsdl/schematron\" queryBinding=\"xslt\"><sch:ns prefix=\"w\" uri=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"/><sch:pattern id=\"p2\"><sch:rule context=\"w:document\"><sch:assert test=\"count(//w:p) = 0\">document should have zero paragraphs</sch:assert></sch:rule></sch:pattern></sch:schema>"})";
  const auto schematronFailResult = invokePythonProvider(cfg, schematronFail);
  if (providerAvailable(probe.providers, "schematron")) {
    require(schematronFailResult.code == PythonBridgeCode::Ok, "schematron fail case should still execute");
    require(schematronFailResult.text.find("\"valid\": false") != std::string::npos,
            "schematron fail case should report valid false");
    require(schematronFailResult.text.find("failed_asserts") != std::string::npos,
            "schematron fail case should expose failed asserts");
  }

  PythonProviderRequest schematronDisallowed = schematronPass;
  schematronDisallowed.payload =
      R"({"part":"word/comments.xml","schema_text":"<sch:schema xmlns:sch=\"http://purl.oclc.org/dsdl/schematron\"><sch:pattern id=\"p\"><sch:rule context=\"*\"><sch:assert test=\"true()\">ok</sch:assert></sch:rule></sch:pattern></sch:schema>"})";
  const auto schematronDisallowedResult = invokePythonProvider(cfg, schematronDisallowed);
  if (providerAvailable(probe.providers, "schematron")) {
    require(schematronDisallowedResult.code == PythonBridgeCode::InvalidRequest,
            "schematron disallowed part should normalize to invalid request");
  }

  PythonProviderRequest schematronInvalid = schematronPass;
  schematronInvalid.payload = R"({"part":"word/document.xml","schema_text":"<sch:schema>"})";
  const auto schematronInvalidResult = invokePythonProvider(cfg, schematronInvalid);
  if (providerAvailable(probe.providers, "schematron")) {
    require(schematronInvalidResult.code == PythonBridgeCode::InvalidRequest,
            "malformed schematron input should normalize to invalid request");
  }

  // PR16: OCR provider (pytesseract + Tesseract runtime).
  PythonProviderRequest ocrRequest;
  ocrRequest.provider = "ocr";
  ocrRequest.operation = "extract_text";
  ocrRequest.payload =
      R"({"image_b64":"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO7Zf6sAAAAASUVORK5CYII=","image_format":"png","lang":"eng","psm":6})";
  const auto ocrResult = invokePythonProvider(cfg, ocrRequest);
  if (providerAvailable(probe.providers, "ocr")) {
    require(ocrResult.code == PythonBridgeCode::Ok, "ocr extract_text should succeed when provider is available");
    require(ocrResult.text.find("\"operation\": \"extract_text\"") != std::string::npos,
            "ocr result should include structured operation metadata");
  } else {
    require(ocrResult.code == PythonBridgeCode::ProviderUnavailable,
            "ocr provider should return provider-unavailable when dependency/runtime is missing");
  }

  PythonProviderRequest ocrMissingInput;
  ocrMissingInput.provider = "ocr";
  ocrMissingInput.operation = "extract_text";
  ocrMissingInput.inputPath = "missing-image.png";
  const auto ocrMissingResult = invokePythonProvider(cfg, ocrMissingInput);
  if (providerAvailable(probe.providers, "ocr")) {
    require(ocrMissingResult.code == PythonBridgeCode::InvalidRequest,
            "ocr missing image input should normalize to invalid request");
  }

  PythonProviderRequest ocrMissingLang = ocrRequest;
  ocrMissingLang.payload =
      R"({"image_b64":"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO7Zf6sAAAAASUVORK5CYII=","image_format":"png","lang":"zzzz","psm":6})";
  const auto ocrMissingLangResult = invokePythonProvider(cfg, ocrMissingLang);
  if (providerAvailable(probe.providers, "ocr")) {
    require(
        ocrMissingLangResult.code == PythonBridgeCode::InvalidRequest ||
            ocrMissingLangResult.code == PythonBridgeCode::ExecutionFailed,
        "ocr missing language data should normalize deterministically");
  }

  // PR18: minimal PDF text extraction provider via pypdf.
  const auto pdfPath = (tempDir / "minidocx_pdf_sample.pdf");
  writeSimplePdfWithText(pdfPath, "Hello PDF");

  PythonProviderRequest pypdfRequest;
  pypdfRequest.provider = "pypdf";
  pypdfRequest.operation = "extract_text";
  pypdfRequest.inputPath = pdfPath.string();
  pypdfRequest.payload = R"({"mode":"plain"})";
  const auto pypdfResult = invokePythonProvider(cfg, pypdfRequest);
  if (providerAvailable(probe.providers, "pypdf")) {
    require(pypdfResult.code == PythonBridgeCode::Ok, "pypdf plain extraction should succeed");
    require(pypdfResult.text.find("\"operation\": \"extract_text\"") != std::string::npos,
            "pypdf response should include operation metadata");
    require(pypdfResult.text.find("Hello PDF") != std::string::npos,
            "pypdf plain extraction should include extracted text");
  } else {
    require(pypdfResult.code == PythonBridgeCode::ProviderUnavailable,
            "pypdf provider should return provider-unavailable when dependency is missing");
  }

  PythonProviderRequest pypdfLayout = pypdfRequest;
  pypdfLayout.payload = R"({"mode":"layout"})";
  const auto pypdfLayoutResult = invokePythonProvider(cfg, pypdfLayout);
  if (providerAvailable(probe.providers, "pypdf")) {
    require(pypdfLayoutResult.code == PythonBridgeCode::Ok, "pypdf layout extraction should succeed");
    require(pypdfLayoutResult.text.find("\"extraction_mode\": \"layout\"") != std::string::npos,
            "pypdf layout extraction should report layout mode");
  }

  PythonProviderRequest pypdfMissingInput;
  pypdfMissingInput.provider = "pypdf";
  pypdfMissingInput.operation = "extract_text";
  pypdfMissingInput.inputPath = "missing.pdf";
  const auto pypdfMissingInputResult = invokePythonProvider(cfg, pypdfMissingInput);
  if (providerAvailable(probe.providers, "pypdf")) {
    require(pypdfMissingInputResult.code == PythonBridgeCode::InvalidRequest,
            "pypdf missing input should normalize to invalid request");
  }

  const auto emptyPdfPath = (tempDir / "minidocx_pdf_empty.pdf");
  writeSimplePdfWithText(emptyPdfPath, "");
  PythonProviderRequest pypdfScannedHint;
  pypdfScannedHint.provider = "pypdf";
  pypdfScannedHint.operation = "extract_text";
  pypdfScannedHint.inputPath = emptyPdfPath.string();
  pypdfScannedHint.payload = R"({"mode":"plain"})";
  const auto pypdfScannedHintResult = invokePythonProvider(cfg, pypdfScannedHint);
  if (providerAvailable(probe.providers, "pypdf")) {
    require(pypdfScannedHintResult.code == PythonBridgeCode::Ok,
            "pypdf empty-text extraction should still complete");
    require(pypdfScannedHintResult.text.find("may require OCR") != std::string::npos,
            "pypdf empty-text extraction should include OCR limitation warning");
  }

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

  std::filesystem::remove(templatePath);
  std::filesystem::remove(renderedPath);
  std::filesystem::remove(pdfPath);
  std::filesystem::remove(emptyPdfPath);
  return 0;
}

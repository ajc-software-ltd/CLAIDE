#include "minidocx/minidocx.hpp"
#include "minidocx/utils/zip.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

enum class FailureKind {
  Packaging,
  Relationships,
  Semantic,
};

struct Failure : std::runtime_error {
  FailureKind kind;
  explicit Failure(FailureKind k, const std::string& msg)
      : std::runtime_error(msg), kind(k) {}
};

std::string kindName(const FailureKind kind) {
  switch (kind) {
    case FailureKind::Packaging: return "packaging failure";
    case FailureKind::Relationships: return "relationship/content-type failure";
    case FailureKind::Semantic: return "semantic mismatch in generated XML";
  }
  return "unknown";
}

void require(const bool condition, const FailureKind kind, const std::string& message) {
  if (!condition) {
    throw Failure(kind, message);
  }
}

std::string readTextFile(const fs::path& path) {
  std::ifstream in(path);
  require(in.good(), FailureKind::Semantic, "cannot read snapshot file: " + path.string());
  std::stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

std::vector<std::string> readSnapshotLines(const fs::path& path) {
  std::ifstream in(path);
  require(in.good(), FailureKind::Semantic, "cannot read snapshot file: " + path.string());
  std::vector<std::string> lines;
  for (std::string line; std::getline(in, line);) {
    if (!line.empty()) {
      lines.push_back(line);
    }
  }
  require(!lines.empty(), FailureKind::Semantic, "snapshot is empty: " + path.string());
  return lines;
}

std::string extractPart(const fs::path& docxPath, const fs::path& partName) {
  md::Zip zip;
  zip.open(docxPath.string(), md::Zip::OpenMode::ReadOnly);
  require(zip.hasEntry(partName), FailureKind::Packaging,
          "missing package part: " + partName.generic_string());
  return zip.extractFileToString(partName);
}

void ensurePartsExist(const fs::path& docxPath, const std::vector<fs::path>& expectedParts) {
  md::Zip zip;
  zip.open(docxPath.string(), md::Zip::OpenMode::ReadOnly);
  for (const auto& part : expectedParts) {
    require(zip.hasEntry(part), FailureKind::Packaging,
            "missing expected package part: " + part.generic_string());
  }
}

void assertSnapshotContains(const std::string& xml, const fs::path& snapshotPath, const FailureKind kind) {
  const auto expected = readSnapshotLines(snapshotPath);
  for (const auto& token : expected) {
    require(xml.find(token) != std::string::npos, kind,
            "snapshot token not found: " + token + " from " + snapshotPath.string());
  }
}

size_t countOccurrences(const std::string& haystack, const std::string& needle) {
  size_t count = 0;
  size_t pos = 0;
  while (true) {
    pos = haystack.find(needle, pos);
    if (pos == std::string::npos) {
      break;
    }
    ++count;
    pos += needle.size();
  }
  return count;
}

fs::path outputRoot() {
  fs::path out = fs::path(MINIDOCX_TEST_OUTPUT_DIR) / "generated";
  fs::create_directories(out);
  return out;
}

fs::path snapshotRoot() {
  return fs::path(MINIDOCX_TESTDATA_DIR) / "snapshots";
}

void verifyRelationshipsAndContentTypes(const fs::path& docxPath) {
  const std::string relsXml = extractPart(docxPath, "/_rels/.rels");
  const std::string contentTypesXml = extractPart(docxPath, "/[Content_Types].xml");
  assertSnapshotContains(relsXml, snapshotRoot() / "relationships.snap", FailureKind::Relationships);
  assertSnapshotContains(contentTypesXml, snapshotRoot() / "content_types.snap", FailureKind::Relationships);
}

void casePlainParagraphs() {
  md::Document doc;
  auto section = doc.addSection();
  section->addParagraph()->addRichText("Plain paragraph one.");
  section->addParagraph()->addRichText("Plain paragraph two.");

  const fs::path out = outputRoot() / "plain.docx";
  doc.saveAs(out.string());

  ensurePartsExist(out, {"/[Content_Types].xml", "/_rels/.rels", "/word/document.xml", "/docProps/core.xml", "/docProps/app.xml"});
  const std::string xml = extractPart(out, "/word/document.xml");
  assertSnapshotContains(xml, snapshotRoot() / "plain_document.snap", FailureKind::Semantic);
  verifyRelationshipsAndContentTypes(out);
}

void caseRichText() {
  md::Document doc;
  auto section = doc.addSection();
  auto paragraph = section->addParagraph();
  auto rich = paragraph->addRichText("BoldItalicUnderlined");
  rich->prop_.fontStyle_.bold_ = true;
  rich->prop_.fontStyle_.italic_ = true;
  rich->prop_.underline_.style_ = md::RichTextProperties::UnderlineStyle::Single;
  rich->prop_.color_ = "FF0000";

  const fs::path out = outputRoot() / "rich_text.docx";
  doc.saveAs(out.string());

  const std::string xml = extractPart(out, "/word/document.xml");
  assertSnapshotContains(xml, snapshotRoot() / "rich_text.snap", FailureKind::Semantic);
  verifyRelationshipsAndContentTypes(out);
}

void caseLists() {
  md::Document doc;
  auto section = doc.addSection();
  const md::NumberingId bulleted = doc.addBulletedListDefinition();
  const md::NumberingId numbered = doc.addNumberedListDefinition();

  auto p1 = section->addParagraph();
  p1->addRichText("Bulleted one");
  p1->numId_ = bulleted;

  auto p2 = section->addParagraph();
  p2->addRichText("Numbered one");
  p2->numId_ = numbered;

  const fs::path out = outputRoot() / "lists.docx";
  doc.saveAs(out.string());

  ensurePartsExist(out, {"/word/numbering.xml"});
  const std::string documentXml = extractPart(out, "/word/document.xml");
  const std::string numberingXml = extractPart(out, "/word/numbering.xml");
  assertSnapshotContains(documentXml, snapshotRoot() / "lists_document.snap", FailureKind::Semantic);
  assertSnapshotContains(numberingXml, snapshotRoot() / "numbering_part.snap", FailureKind::Semantic);
}

void caseTablesAndMerges() {
  md::Document doc;
  auto section = doc.addSection();
  auto table = section->addTable(4, 4);

  table->cellAt(0, 0)->addParagraph()->addRichText("Merged A");
  table->merge(0, 0, 1, 2);
  table->merge(1, 1, 2, 1);

  const fs::path out = outputRoot() / "tables.docx";
  doc.saveAs(out.string());

  const std::string xml = extractPart(out, "/word/document.xml");
  assertSnapshotContains(xml, snapshotRoot() / "table_document.snap", FailureKind::Semantic);
}

void casePictures() {
  md::Document doc;
  auto section = doc.addSection();

  const md::Buffer png = {
      0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
      0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
      0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
      0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
      0x89, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x44, 0x41,
      0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
      0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00,
      0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
      0x42, 0x60, 0x82,
  };

  auto picture = section->addParagraph()->addPicture(doc.addImage(png, md::FileType::PNG));
  picture->prop_.extent_.setSize(4725, 3173, 300, 20);

  const fs::path out = outputRoot() / "picture.docx";
  doc.saveAs(out.string());

  ensurePartsExist(out, {"/word/media/image1.png"});
  const std::string xml = extractPart(out, "/word/document.xml");
  assertSnapshotContains(xml, snapshotRoot() / "picture_document.snap", FailureKind::Semantic);
}

void caseStyles() {
  md::Document doc;
  auto section = doc.addSection();

  md::ParagraphStyle paragraphStyle;
  paragraphStyle.name_ = "My Heading";
  paragraphStyle.outlineLevel_ = md::ParagraphProperties::OutlineLevel::Level1;
  paragraphStyle.fontSize_ = 30;
  doc.addParagraphStyle(paragraphStyle);

  md::CharacterStyle characterStyle;
  characterStyle.name_ = "My Character";
  characterStyle.color_ = "336699";
  characterStyle.fontStyle_.bold_ = true;
  doc.addCharacterStyle(characterStyle);

  auto paragraph = section->addParagraph();
  paragraph->prop_.style_ = "My Heading";
  auto rich = paragraph->addRichText("Styled text");
  rich->prop_.style_ = "My Character";

  const fs::path out = outputRoot() / "styles.docx";
  doc.saveAs(out.string());

  ensurePartsExist(out, {"/word/styles.xml"});
  const std::string stylesXml = extractPart(out, "/word/styles.xml");
  assertSnapshotContains(stylesXml, snapshotRoot() / "styles_part.snap", FailureKind::Semantic);
}

void caseMultiSection() {
  md::Document doc;
  auto section1 = doc.addSection();
  auto section2 = doc.addSection();
  section1->prop_.landscape_ = false;
  section2->prop_.landscape_ = true;

  section1->addParagraph()->addRichText("Section one");
  section2->addParagraph()->addRichText("Section two");

  const fs::path out = outputRoot() / "multisection.docx";
  doc.saveAs(out.string());

  const std::string xml = extractPart(out, "/word/document.xml");
  require(countOccurrences(xml, "<w:sectPr") >= 2, FailureKind::Semantic,
          "expected at least two section property blocks");
}

void caseSmokeReopenParts() {
  md::Document doc;
  auto section = doc.addSection();
  section->addParagraph()->addRichText("Smoke document");
  const fs::path out = outputRoot() / "smoke.docx";
  doc.saveAs(out.string());

  md::Zip zip;
  zip.open(out.string(), md::Zip::OpenMode::ReadOnly);
  require(zip.countEntries() > 0, FailureKind::Packaging, "zip has no entries");
  require(zip.hasEntry("/[Content_Types].xml"), FailureKind::Packaging, "missing content types");
  require(zip.hasEntry("/_rels/.rels"), FailureKind::Relationships, "missing package relationships");
  require(zip.hasEntry("/word/document.xml"), FailureKind::Packaging, "missing document xml");
}

void runCase(const std::string& name, const std::function<void()>& fn) {
  try {
    fn();
    std::cout << "[PASS] " << name << '\n';
  } catch (const Failure& ex) {
    std::cerr << "[FAIL] " << name << ": " << kindName(ex.kind) << " -> " << ex.what() << '\n';
    throw;
  }
}

}

int main() {
  const std::vector<std::pair<std::string, std::function<void()>>> cases = {
      {"plain paragraphs", casePlainParagraphs},
      {"rich text formatting", caseRichText},
      {"numbered and bulleted lists", caseLists},
      {"tables and merged cells", caseTablesAndMerges},
      {"pictures", casePictures},
      {"paragraph and character styles", caseStyles},
      {"multi-section document", caseMultiSection},
      {"create-save-reopen package parts", caseSmokeReopenParts},
  };

  for (const auto& [name, fn] : cases) {
    runCase(name, fn);
  }

  std::cout << "minidocx regression suite completed" << std::endl;
  return 0;
}

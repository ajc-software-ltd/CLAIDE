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

void caseLoadSaveReloadRoundTrip() {
  md::Document source;
  auto section = source.addSection();
  auto p = section->addParagraph();
  p->prop_.style_ = "Heading1";
  p->addRichText("Round trip");
  p->addRichText("\nwith break\tand tab");
  const md::NumberingId listId = source.addNumberedListDefinition();
  p->numId_ = listId;
  p->level_ = md::NumberingLevel::Level2;
  section->addTable(2, 2)->merge(0, 0, 2, 1);

  const fs::path original = outputRoot() / "roundtrip_original.docx";
  const fs::path saved = outputRoot() / "roundtrip_saved.docx";
  source.saveAs(original.string());

  md::Document loaded;
  loaded.load(original.string());
  loaded.saveAs(saved.string());

  md::Document reloaded;
  reloaded.load(saved.string());

  const auto sections = reloaded.sections();
  require(!sections.empty(), FailureKind::Semantic, "expected non-empty sections after reload");
  const auto blocks = sections.front()->blocks();
  require(blocks.size() >= 2, FailureKind::Semantic, "expected paragraph+table after reload");

  auto para = std::dynamic_pointer_cast<md::Paragraph>(blocks.front());
  require(static_cast<bool>(para), FailureKind::Semantic, "expected first block paragraph after reload");
  require(para->numId_ > 0, FailureKind::Semantic, "expected numbering linkage after reload");
  require(!para->runs().empty(), FailureKind::Semantic, "expected rich text runs after reload");

  auto table = std::dynamic_pointer_cast<md::Table>(blocks.back());
  require(static_cast<bool>(table), FailureKind::Semantic, "expected second block table after reload");
  const auto mergedCell = table->cellAt(0, 0);
  require(mergedCell->rect().rows() == 2 && mergedCell->rect().cols() == 1,
          FailureKind::Semantic, "expected merged cell preserved after reload");
}

void caseParagraphPageBreakAfterRoundTrip() {
  md::Document doc;
  auto section = doc.addSection();
  auto paragraph = section->addParagraph();
  paragraph->prop_.pageBreakAfter_ = true;
  paragraph->addRichText("Page break after");

  const fs::path out = outputRoot() / "page_break_after.docx";
  doc.saveAs(out.string());
  const std::string xml = extractPart(out, "/word/document.xml");
  require(xml.find("<w:pageBreakAfter") != std::string::npos, FailureKind::Semantic,
          "pageBreakAfter element not serialized");

  md::Document loaded;
  loaded.load(out.string());
  const auto blocks = loaded.sections().front()->blocks();
  auto para = std::dynamic_pointer_cast<md::Paragraph>(blocks.front());
  require(static_cast<bool>(para) && para->prop_.pageBreakAfter_, FailureKind::Semantic,
          "pageBreakAfter did not round-trip");
}

void caseSectionDocGridRoundTrip() {
  md::Document doc;
  auto section = doc.addSection();
  section->prop_.type_ = md::SectionProperties::Type::NextPage;
  md::SectionProperties::DocGrid grid;
  grid.type_ = md::SectionProperties::DocGrid::Type::LinesAndChars;
  grid.linePitch_ = 420;
  section->prop_.docGrid_ = grid;
  section->addParagraph()->addRichText("Grid test");

  const fs::path out = outputRoot() / "section_doc_grid.docx";
  doc.saveAs(out.string());
  const std::string xml = extractPart(out, "/word/document.xml");
  require(xml.find("w:type w:val=\"nextPage\"") != std::string::npos, FailureKind::Semantic,
          "section type not serialized");
  require(xml.find("w:docGrid") != std::string::npos, FailureKind::Semantic,
          "docGrid element not serialized");
  require(xml.find("w:type=\"linesAndChars\"") != std::string::npos, FailureKind::Semantic,
          "docGrid type not serialized");
  require(xml.find("w:linePitch=\"420\"") != std::string::npos, FailureKind::Semantic,
          "docGrid linePitch not serialized");

  md::Document loaded;
  loaded.load(out.string());
  const auto loadedSections = loaded.sections();
  require(!loadedSections.empty(), FailureKind::Semantic, "missing sections after doc-grid reload");
  const auto& loadedSection = loadedSections.front();
  require(loadedSection->prop_.type_ == md::SectionProperties::Type::NextPage, FailureKind::Semantic,
          "section type did not round-trip");
  require(loadedSection->prop_.docGrid_.has_value(), FailureKind::Semantic,
          "docGrid missing after round-trip");
  require(loadedSection->prop_.docGrid_->type_ == md::SectionProperties::DocGrid::Type::LinesAndChars,
          FailureKind::Semantic, "docGrid type did not round-trip");
  require(loadedSection->prop_.docGrid_->linePitch_ == 420, FailureKind::Semantic,
          "docGrid linePitch did not round-trip");
}

void casePictureCropStretchRoundTrip() {
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
  md::PictureProperties::Cropping crop{1, 2, 3, 4};
  md::PictureProperties::Stretching stretch{5, 6, 7, 8};
  picture->prop_.cropping_ = crop;
  picture->prop_.stretching_ = stretch;

  const fs::path out = outputRoot() / "picture_crop_stretch.docx";
  doc.saveAs(out.string());
  const std::string xml = extractPart(out, "/word/document.xml");
  require(xml.find("a:srcRect") != std::string::npos, FailureKind::Semantic,
          "srcRect not serialized");
  require(xml.find("a:fillRect") != std::string::npos, FailureKind::Semantic,
          "fillRect not serialized");
  require(xml.find("t=\"1\"") != std::string::npos, FailureKind::Semantic,
          "crop top not serialized");
  require(xml.find("r=\"8\"") != std::string::npos, FailureKind::Semantic,
          "stretch right not serialized");

  md::Document loaded;
  loaded.load(out.string());
  const auto loadedSections = loaded.sections();
  require(!loadedSections.empty(), FailureKind::Semantic, "missing sections after picture reload");
  const auto blocks = loadedSections.front()->blocks();
  require(!blocks.empty(), FailureKind::Semantic, "missing paragraph after picture reload");
  auto para = std::dynamic_pointer_cast<md::Paragraph>(blocks.front());
  require(static_cast<bool>(para), FailureKind::Semantic, "first block is not paragraph after picture reload");
  require(!para->runs().empty(), FailureKind::Semantic, "missing picture run after reload");
  auto run = para->runs().back();
  auto loadedPicture = std::dynamic_pointer_cast<md::Picture>(run);
  require(static_cast<bool>(loadedPicture), FailureKind::Semantic, "picture missing after reload");
  require(loadedPicture->prop_.cropping_.has_value(), FailureKind::Semantic, "crop missing after reload");
  require(loadedPicture->prop_.stretching_.has_value(), FailureKind::Semantic, "stretch missing after reload");
  require(loadedPicture->prop_.cropping_->left_ == 3, FailureKind::Semantic, "crop left mismatch after reload");
  require(loadedPicture->prop_.stretching_->bottom_ == 6, FailureKind::Semantic, "stretch bottom mismatch after reload");
}

void caseNumberingStylesAndOverridesRoundTrip() {
  md::Document doc;
  auto section = doc.addSection();

  md::AbstractNumberingDefinition abstractDef;
  abstractDef.type_ = md::NumberingType::MultiLevel;
  abstractDef.levels_[0].numStyle_ = md::NumberStyle::UpperRoman;
  abstractDef.levels_[0].numFmt_ = "%1.";
  abstractDef.levels_[1].numStyle_ = md::NumberStyle::UpperLetter;
  abstractDef.levels_[1].numFmt_ = "%2)";
  abstractDef.levels_[2].numStyle_ = md::NumberStyle::OrdinalText;
  abstractDef.levels_[2].numFmt_ = "%3";
  abstractDef.levels_[3].numStyle_ = md::NumberStyle::CardinalText;
  abstractDef.levels_[3].numFmt_ = "%4";

  const md::NumberingId abstractId = doc.addAbstractNumDefinition(abstractDef);
  md::NumberingDefinition def(abstractId);
  md::LevelDefinition overrideLevel;
  overrideLevel.numStart_ = 5;
  overrideLevel.numStyle_ = md::NumberStyle::LowerLetter;
  overrideLevel.numFmt_ = "(%1)";
  def.levelOverrides_[md::NumberingLevel::Level1] = overrideLevel;
  const md::NumberingId numId = doc.addNumDefinition(def);

  auto p = section->addParagraph();
  p->numId_ = numId;
  p->addRichText("override list");

  const fs::path out = outputRoot() / "numbering_overrides.docx";
  doc.saveAs(out.string());
  const std::string xml = extractPart(out, "/word/numbering.xml");
  require(xml.find("w:val=\"upperRoman\"") != std::string::npos, FailureKind::Semantic,
          "upperRoman not serialized");
  require(xml.find("w:val=\"upperLetter\"") != std::string::npos, FailureKind::Semantic,
          "upperLetter not serialized");
  require(xml.find("w:val=\"ordinalText\"") != std::string::npos, FailureKind::Semantic,
          "ordinalText not serialized");
  require(xml.find("w:val=\"cardinalText\"") != std::string::npos, FailureKind::Semantic,
          "cardinalText not serialized");
  require(xml.find("w:lvlOverride") != std::string::npos, FailureKind::Semantic,
          "level override not serialized");
  require(xml.find("w:startOverride") != std::string::npos, FailureKind::Semantic,
          "start override not serialized");

  md::Document loaded;
  loaded.load(out.string());
  const auto defs = loaded.numberingDefinitions();
  require(defs.find(numId) != defs.end(), FailureKind::Semantic, "numbering definition missing after reload");
  const auto& loadedDef = defs.at(numId);
  require(loadedDef.levelOverrides_.find(md::NumberingLevel::Level1) != loadedDef.levelOverrides_.end(),
          FailureKind::Semantic, "level override missing after reload");
  require(loadedDef.levelOverrides_.at(md::NumberingLevel::Level1).numStart_ == 5,
          FailureKind::Semantic, "level override start mismatch after reload");
}

void caseRelationshipTargetDeduplication() {
  md::Document doc;
  auto section = doc.addSection();
  section->addParagraph()->addRichText("relationship dedup");
  doc.addNumberedListDefinition();
  doc.addNumberedListDefinition();

  const fs::path out = outputRoot() / "relationships_dedup.docx";
  doc.saveAs(out.string());
  const std::string relsXml = extractPart(out, "/word/_rels/document.xml.rels");
  require(countOccurrences(relsXml, "Target=\"numbering.xml\"") == 1, FailureKind::Relationships,
          "numbering target relationship should be unique");
}

void caseImportFromGeneratedCorpus() {
  casePlainParagraphs();
  caseRichText();
  caseLists();
  caseTablesAndMerges();
  casePictures();
  caseStyles();
  caseMultiSection();

  const std::vector<fs::path> docs = {
      outputRoot() / "plain.docx",
      outputRoot() / "rich_text.docx",
      outputRoot() / "lists.docx",
      outputRoot() / "tables.docx",
      outputRoot() / "picture.docx",
      outputRoot() / "styles.docx",
      outputRoot() / "multisection.docx",
  };

  for (const auto& path : docs) {
    md::Document loaded;
    loaded.load(path.string());
    const auto sections = loaded.sections();
    require(!sections.empty(), FailureKind::Semantic, "import produced empty section list: " + path.string());
    const fs::path resaved = outputRoot() / ("resave_" + path.filename().string());
    loaded.saveAs(resaved.string());
    ensurePartsExist(resaved, {"/[Content_Types].xml", "/_rels/.rels", "/word/document.xml"});
  }
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
      {"load-save-reload round trip", caseLoadSaveReloadRoundTrip},
      {"paragraph page-break-after round trip", caseParagraphPageBreakAfterRoundTrip},
      {"section doc-grid round trip", caseSectionDocGridRoundTrip},
      {"picture crop/stretch round trip", casePictureCropStretchRoundTrip},
      {"numbering styles and overrides round trip", caseNumberingStylesAndOverridesRoundTrip},
      {"relationship target deduplication", caseRelationshipTargetDeduplication},
      {"import from generated corpus", caseImportFromGeneratedCorpus},
  };

  for (const auto& [name, fn] : cases) {
    runCase(name, fn);
  }

  std::cout << "minidocx regression suite completed" << std::endl;
  return 0;
}

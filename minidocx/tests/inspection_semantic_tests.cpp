#include "minidocx/minidocx.hpp"

#include <stdexcept>
#include <string>

namespace
{
void require(const bool condition, const std::string& message)
{
  if (!condition)
    throw std::runtime_error(message);
}

md::Document buildDocument()
{
  md::Document doc;

  auto section = doc.addSection();
  auto heading = section->addParagraph();
  heading->prop_.outlineLevel_ = md::ParagraphProperties::OutlineLevel::Level1;
  heading->addRichText("Heading");

  const auto bulletListId = doc.addBulletedListDefinition();
  auto listItem = section->addParagraph();
  listItem->numId_ = bulletListId;
  listItem->addRichText("List item");

  auto table = section->addTable(2, 2);
  table->cellAt(0, 0)->addParagraph()->addRichText("A");
  table->cellAt(0, 1)->addParagraph()->addRichText("B");
  table->cellAt(1, 0)->addParagraph()->addRichText("C");
  table->cellAt(1, 1)->addParagraph()->addRichText("D");
  table->merge(0, 0, 1, 2);

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

  auto pictureParagraph = section->addParagraph();
  const auto imageId = doc.addImage(png, md::FileType::PNG);
  auto picture = pictureParagraph->addPicture(imageId);
  picture->prop_.extent_.setSize(128, 64, 96, 96);

  return doc;
}

void testSummarizeAndListing()
{
  const md::Document doc = buildDocument();

  const auto stats = md::inspection::summarize(doc);
  require(stats.sectionCount == 1, "section count mismatch");
  require(stats.paragraphCount == 3, "paragraph count mismatch");
  require(stats.runCount == 3, "run count mismatch");
  require(stats.tableCount == 1, "table count mismatch");
  require(stats.cellCount == 3, "cell count should count merged cells once");
  require(stats.pictureCount == 1, "picture count mismatch");

  const auto sections = md::inspection::listSections(doc);
  require(sections.size() == 1, "section listing size mismatch");
  require(sections[0].paragraphCount == 3, "section paragraph count mismatch");

  const auto paragraphs = md::inspection::listParagraphs(doc);
  require(paragraphs.size() == 3, "paragraph listing size mismatch");
  require(paragraphs[0].visibleText == "Heading", "heading text mismatch");

  const auto outlines = md::inspection::listOutlineParagraphs(doc);
  require(outlines.size() == 1, "outline paragraph count mismatch");
  require(outlines[0].visibleText == "Heading", "outline paragraph text mismatch");

  const auto runs = md::inspection::listRuns(doc);
  require(runs.size() == 3, "run listing size mismatch");

  const auto tables = md::inspection::listTables(doc);
  require(tables.size() == 1, "table listing size mismatch");
  require(tables[0].rows == 2 && tables[0].cols == 2, "table shape mismatch");

  const auto cells = md::inspection::listCells(doc);
  require(cells.size() == 3, "cell listing size mismatch");

  const auto pictures = md::inspection::listPictures(doc);
  require(pictures.size() == 1, "picture listing size mismatch");
  require(pictures[0].relationshipId > 0, "picture relationship id mismatch");
  require(pictures[0].width > 0 && pictures[0].height > 0, "picture dimensions mismatch");

  require(md::inspection::hasNumbering(doc), "numbering should exist");
}

void testVisibleTextExtraction()
{
  const md::Document doc = buildDocument();

  const std::string allText = md::inspection::extractVisibleText(doc);
  require(allText.find("Heading") != std::string::npos, "missing heading from visible text");
  require(allText.find("List item") != std::string::npos, "missing list item from visible text");
  require(allText.find("A") != std::string::npos, "missing table text from visible text");

  const std::string sectionText = md::inspection::extractSectionVisibleText(doc, 0);
  require(sectionText == allText, "single-section text should match full text");

  const std::string missingSectionText = md::inspection::extractSectionVisibleText(doc, 99);
  require(missingSectionText.empty(), "out-of-range section text should be empty");
}

void testComputedStyleResolutionPrecedence()
{
  md::Document doc;
  auto section = doc.addSection();

  md::ParagraphStyle baseParagraphStyle;
  baseParagraphStyle.name_ = "Base Heading";
  baseParagraphStyle.outlineLevel_ = md::ParagraphProperties::OutlineLevel::Level3;
  baseParagraphStyle.fontSize_ = 24;
  baseParagraphStyle.color_ = "112233";
  doc.addParagraphStyle(baseParagraphStyle);

  md::ParagraphStyle derivedParagraphStyle;
  derivedParagraphStyle.name_ = "Derived Heading";
  derivedParagraphStyle.basedOn_ = "BaseHeading";
  derivedParagraphStyle.align_ = md::Alignment::Right;
  doc.addParagraphStyle(derivedParagraphStyle);

  md::CharacterStyle baseCharacterStyle;
  baseCharacterStyle.name_ = "Base Char";
  baseCharacterStyle.fontStyle_.italic_ = true;
  baseCharacterStyle.color_ = "445566";
  doc.addCharacterStyle(baseCharacterStyle);

  md::CharacterStyle derivedCharacterStyle;
  derivedCharacterStyle.name_ = "Derived Char";
  derivedCharacterStyle.basedOn_ = "BaseChar";
  derivedCharacterStyle.fontStyle_.bold_ = true;
  doc.addCharacterStyle(derivedCharacterStyle);

  const md::NumberingId listId = doc.addNumberedListDefinition();
  auto paragraph = section->addParagraph();
  paragraph->prop_.style_ = "Derived Heading";
  paragraph->numId_ = listId;
  paragraph->level_ = md::NumberingLevel::Level2;
  paragraph->prop_.align_ = md::Alignment::Centered;

  auto run = paragraph->addRichText("Resolved");
  run->prop_.style_ = "Derived Char";
  run->prop_.fontStyle_.italic_ = false; // represented as default/unset in current model
  run->prop_.fontSize_ = 30;

  const auto resolvedParagraph = md::inspection::resolveParagraphFormatting(doc, *paragraph);
  require(resolvedParagraph.paragraphStyleId == "DerivedHeading", "paragraph style id should be normalized");
  require(resolvedParagraph.paragraphStyleChain.size() == 2, "paragraph style chain depth mismatch");
  require(resolvedParagraph.paragraph.align_.has_value(), "paragraph alignment should resolve");
  require(resolvedParagraph.paragraph.align_.value() == md::Alignment::Centered,
          "direct paragraph alignment should override style alignment");
  require(resolvedParagraph.paragraph.outlineLevel_ == md::ParagraphProperties::OutlineLevel::Level3,
          "outline level should be inherited from basedOn style");
  require(resolvedParagraph.headingLike, "resolved heading flag should be true");
  require(resolvedParagraph.list.hasNumbering, "list formatting should resolve");
  require(resolvedParagraph.list.level == md::NumberingLevel::Level2, "resolved list level mismatch");
  require(!resolvedParagraph.list.format.empty(), "resolved list format should be present");

  const auto resolvedRun = md::inspection::resolveRunFormatting(doc, *paragraph, *run);
  require(resolvedRun.characterStyleId == "DerivedChar", "character style id should be normalized");
  require(resolvedRun.characterStyleChain.size() == 2, "character style chain depth mismatch");
  require(resolvedRun.run.fontStyle_.bold_, "bold should be inherited from derived character style");
  require(resolvedRun.run.fontStyle_.italic_, "italic should be inherited from basedOn character style");
  require(resolvedRun.run.fontSize_ == 30, "direct run font size should override paragraph style");
}

void testComputedStyleResolutionFailuresAndDeterminism()
{
  md::Document doc;
  auto section = doc.addSection();

  md::ParagraphStyle cycleParagraphStyleA;
  cycleParagraphStyleA.name_ = "Cycle A";
  cycleParagraphStyleA.basedOn_ = "CycleB";
  doc.addParagraphStyle(cycleParagraphStyleA);

  md::ParagraphStyle cycleParagraphStyleB;
  cycleParagraphStyleB.name_ = "Cycle B";
  cycleParagraphStyleB.basedOn_ = "CycleA";
  doc.addParagraphStyle(cycleParagraphStyleB);

  md::CharacterStyle cycleCharacterStyleA;
  cycleCharacterStyleA.name_ = "Cycle Char A";
  cycleCharacterStyleA.basedOn_ = "CycleCharB";
  doc.addCharacterStyle(cycleCharacterStyleA);

  md::CharacterStyle cycleCharacterStyleB;
  cycleCharacterStyleB.name_ = "Cycle Char B";
  cycleCharacterStyleB.basedOn_ = "CycleCharA";
  doc.addCharacterStyle(cycleCharacterStyleB);

  auto missingStyleParagraph = section->addParagraph();
  missingStyleParagraph->prop_.style_ = "Missing Paragraph";
  missingStyleParagraph->numId_ = 42;
  auto missingStyleRun = missingStyleParagraph->addRichText("Missing");
  missingStyleRun->prop_.style_ = "Missing Run";

  auto cycleParagraph = section->addParagraph();
  cycleParagraph->prop_.style_ = "Cycle A";
  auto cycleRun = cycleParagraph->addRichText("Cycle");
  cycleRun->prop_.style_ = "Cycle Char A";

  const auto missingResolvedParagraphA = md::inspection::resolveParagraphFormatting(doc, *missingStyleParagraph);
  const auto missingResolvedParagraphB = md::inspection::resolveParagraphFormatting(doc, *missingStyleParagraph);
  require(missingResolvedParagraphA.issues == missingResolvedParagraphB.issues, "paragraph resolution should be deterministic");
  require(!missingResolvedParagraphA.issues.empty(), "missing references should produce resolve issues");

  const auto missingResolvedRun = md::inspection::resolveRunFormatting(doc, *missingStyleParagraph, *missingStyleRun);
  require(!missingResolvedRun.issues.empty(), "missing run style should produce resolve issues");

  const auto cycleResolvedParagraph = md::inspection::resolveParagraphFormatting(doc, *cycleParagraph);
  require(!cycleResolvedParagraph.issues.empty(), "paragraph style cycle should produce resolve issues");

  const auto cycleResolvedRun = md::inspection::resolveRunFormatting(doc, *cycleParagraph, *cycleRun);
  require(!cycleResolvedRun.issues.empty(), "character style cycle should produce resolve issues");
}

void testLayoutSinglePageAndGeometry()
{
  md::Document doc;
  auto section = doc.addSection();
  section->prop_.size_.width_ = md::A4_W;
  section->prop_.size_.height_ = md::A4_H;
  section->prop_.margins_.left_ = 1000;
  section->prop_.margins_.right_ = 1000;
  section->prop_.margins_.top_ = 1200;
  section->prop_.margins_.bottom_ = 1200;

  auto paragraph = section->addParagraph();
  paragraph->addRichText("Simple layout paragraph");

  auto table = section->addTable(2, 2);
  table->cellAt(0, 0)->addParagraph()->addRichText("A");
  table->cellAt(0, 1)->addParagraph()->addRichText("B");
  table->cellAt(1, 0)->addParagraph()->addRichText("C");
  table->cellAt(1, 1)->addParagraph()->addRichText("D");

  const auto layout = md::inspection::buildLayout(doc);
  require(!layout.pages.empty(), "layout should contain at least one page");
  require(layout.pages[0].pageRect.width == md::A4_W, "page width mismatch");
  require(layout.pages[0].pageRect.height == md::A4_H, "page height mismatch");
  require(layout.pages[0].contentRect.x == 1000, "content x mismatch");
  require(layout.pages[0].contentRect.y == 1200, "content y mismatch");
  require(layout.pages[0].contentRect.width == (md::A4_W - 2000), "content width mismatch");
  require(layout.pages[0].contentRect.height == (md::A4_H - 2400), "content height mismatch");

  bool hasParagraph = false;
  bool hasTable = false;
  bool hasCell = false;
  for (const auto& node : layout.nodes) {
    if (node.kind == md::inspection::LayoutNodeKind::Paragraph)
      hasParagraph = true;
    if (node.kind == md::inspection::LayoutNodeKind::Table)
      hasTable = true;
    if (node.kind == md::inspection::LayoutNodeKind::Cell)
      hasCell = true;
  }

  require(hasParagraph, "layout should include paragraph node");
  require(hasTable, "layout should include table node");
  require(hasCell, "layout should include cell node");
}

void testLayoutMultiPageAndDeterminism()
{
  md::Document doc;
  auto section = doc.addSection();
  section->prop_.margins_.top_ = 600;
  section->prop_.margins_.bottom_ = 600;
  section->prop_.margins_.left_ = 600;
  section->prop_.margins_.right_ = 600;

  auto paragraph = section->addParagraph();
  for (size_t i = 0; i < 220; i++)
    paragraph->addRichText("This is a long line intended to force pagination.\n");

  const auto layoutA = md::inspection::buildLayout(doc);
  const auto layoutB = md::inspection::buildLayout(doc);

  require(layoutA.pages.size() >= 2, "long content should span multiple pages");
  require(layoutA.pages.size() == layoutB.pages.size(), "page count should be deterministic");
  require(layoutA.nodes.size() == layoutB.nodes.size(), "node count should be deterministic");
  require(layoutA.pages[0].lines.size() == layoutB.pages[0].lines.size(), "line count should be deterministic");
}

void testLayoutPicturePlacementAndMapping()
{
  md::Document doc;
  auto section = doc.addSection();
  auto paragraph = section->addParagraph();
  paragraph->addRichText("Before image");

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

  const auto imageId = doc.addImage(png, md::FileType::PNG);
  auto picture = paragraph->addPicture(imageId);
  picture->prop_.extent_.setSize(192, 96, 96, 100);

  const auto layout = md::inspection::buildLayout(doc);
  require(!layout.nodes.empty(), "layout node list should not be empty");

  bool hasPictureNode = false;
  bool hasLineNode = false;
  for (const auto& node : layout.nodes) {
    if (node.kind == md::inspection::LayoutNodeKind::Picture) {
      hasPictureNode = true;
      require(node.hasNodePath, "picture node should include semantic node path");
      require(node.rect.width > 0 && node.rect.height > 0, "picture rect dimensions should be positive");
    }
    if (node.kind == md::inspection::LayoutNodeKind::Line)
      hasLineNode = true;
  }

  require(hasPictureNode, "layout should include picture node");
  require(hasLineNode, "layout should include line node");
}
}

int main()
{
  testSummarizeAndListing();
  testVisibleTextExtraction();
  testComputedStyleResolutionPrecedence();
  testComputedStyleResolutionFailuresAndDeterminism();
  testLayoutSinglePageAndGeometry();
  testLayoutMultiPageAndDeterminism();
  testLayoutPicturePlacementAndMapping();
  return 0;
}

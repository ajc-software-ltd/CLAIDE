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
}

int main()
{
  testSummarizeAndListing();
  testVisibleTextExtraction();
  return 0;
}

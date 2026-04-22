#include "minidocx/minidocx.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
void require(const bool condition, const std::string& message)
{
  if (!condition)
    throw std::runtime_error(message);
}

md::Buffer tinyPng()
{
  return {
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
}

void testCommandsSuccessFlow()
{
  md::Document doc;

  auto section = doc.addSection();
  auto paragraph = section->addParagraph();
  paragraph->addRichText("Initial text");

  md::ParagraphStyle paragraphStyle;
  paragraphStyle.name_ = "Body Style";
  doc.addParagraphStyle(paragraphStyle);

  md::CharacterStyle characterStyle;
  characterStyle.name_ = "Inline Style";
  doc.addCharacterStyle(characterStyle);

  const auto numberedListId = doc.addNumberedListDefinition();

  auto result = md::editing::applyCommand(doc, md::editing::InsertSectionCommand{1});
  require(result.success, "insert section command should succeed");
  require(doc.sectionCount() == 2, "section count should increase");

  result = md::editing::applyCommand(doc, md::editing::DeleteSectionCommand{1});
  require(result.success, "delete section command should succeed");
  require(doc.sectionCount() == 1, "section count should decrease");

  result = md::editing::applyCommand(doc, md::editing::InsertParagraphCommand{0, 0});
  require(result.success, "insert paragraph command should succeed");
  require(section->blockCount() == 2, "block count should increase");

  result = md::editing::applyCommand(doc, md::editing::ReplaceParagraphTextCommand{{0, 0, 0, false}, "Paragraph replaced"});
  require(result.success, "replace paragraph text command should succeed");

  result = md::editing::applyCommand(doc, md::editing::InsertRichTextRunCommand{{0, 0, 0, false}, 1, " second"});
  require(result.success, "insert rich text run command should succeed");

  result = md::editing::applyCommand(doc, md::editing::ReplaceRunTextCommand{{0, 0, 1, true}, " third"});
  require(result.success, "replace run text command should succeed");

  result = md::editing::applyCommand(doc, md::editing::ApplyParagraphStyleCommand{{0, 0, 0, false}, "Body Style"});
  require(result.success, "apply paragraph style command should succeed");

  result = md::editing::applyCommand(doc, md::editing::ApplyCharacterStyleCommand{{0, 0, 0, true}, "Inline Style"});
  require(result.success, "apply character style command should succeed");

  md::ParagraphProperties paragraphProperties;
  paragraphProperties.keepNext_ = true;
  paragraphProperties.align_ = md::Alignment::Justified;
  result = md::editing::applyCommand(doc, md::editing::SetParagraphPropertiesCommand{{0, 0, 0, false}, paragraphProperties});
  require(result.success, "set paragraph properties command should succeed");

  md::RichTextProperties runProperties;
  runProperties.fontSize_ = 28;
  runProperties.color_ = "0099CC";
  result = md::editing::applyCommand(doc, md::editing::SetRunPropertiesCommand{{0, 0, 0, true}, runProperties});
  require(result.success, "set run properties command should succeed");

  result = md::editing::applyCommand(doc, md::editing::ApplyNumberingCommand{{0, 0, 0, false}, numberedListId, md::NumberingLevel::Level2});
  require(result.success, "apply numbering command should succeed");

  result = md::editing::applyCommand(doc, md::editing::InsertPictureCommand{{0, 0, 0, false}, 1, tinyPng(), md::FileType::PNG});
  require(result.success, "insert picture command should succeed");

  result = md::editing::applyCommand(doc, md::editing::CreateTableCommand{0, 1, 2, 2});
  require(result.success, "create table command should succeed");

  result = md::editing::applyCommand(doc, md::editing::MergeCellsCommand{{0, 1, 0, false}, 0, 0, 1, 2});
  require(result.success, "merge cells command should succeed");

  result = md::editing::applyCommand(doc, md::editing::SplitCellCommand{{0, 1, 0, false}, 0, 0});
  require(result.success, "split cell command should succeed");

  md::SectionProperties sectionProperties;
  sectionProperties.landscape_ = true;
  sectionProperties.margins_.left_ = 900;
  result = md::editing::applyCommand(doc, md::editing::UpdateSectionPropertiesCommand{0, sectionProperties});
  require(result.success, "update section properties command should succeed");

  result = md::editing::applyCommand(doc, md::editing::DeleteBlockCommand{0, 1});
  require(result.success, "delete block command should succeed");

  const auto text = md::inspection::extractVisibleText(doc);
  require(text.find("Paragraph replaced") != std::string::npos, "visible text should include replaced paragraph text");

  const auto resolved = md::inspection::resolveParagraphFormatting(doc, *std::dynamic_pointer_cast<md::Paragraph>(section->blockAt(0)));
  require(resolved.paragraph.keepNext_, "resolved paragraph should include command-applied paragraph properties");

  const auto layout = md::inspection::buildLayout(doc);
  require(!layout.pages.empty(), "layout should still build after command sequence");

  std::stringstream stream;
  doc.saveToStream(stream);
  md::Document reloaded;
  reloaded.loadFromStream(stream);
  require(!md::inspection::extractVisibleText(reloaded).empty(), "round-trip document should remain readable");
}

void testCommandFailures()
{
  md::Document doc;
  auto section = doc.addSection();
  auto paragraph = section->addParagraph();
  paragraph->addRichText("Failure test");

  auto result = md::editing::applyCommand(doc, md::editing::DeleteSectionCommand{5});
  require(!result.success && result.error == md::editing::CommandError::InvalidPath,
          "delete section should fail with invalid path");

  result = md::editing::applyCommand(doc, md::editing::DeleteSectionCommand{0});
  require(!result.success && result.error == md::editing::CommandError::OperationNotAllowed,
          "delete only section should fail");

  result = md::editing::applyCommand(doc, md::editing::ReplaceRunTextCommand{{0, 0, 2, true}, "x"});
  require(!result.success && result.error == md::editing::CommandError::InvalidNodeType,
          "replace run text should fail for out-of-range run");

  result = md::editing::applyCommand(doc, md::editing::CreateTableCommand{0, 0, 0, 2});
  require(!result.success && result.error == md::editing::CommandError::InvalidArgument,
          "create table should fail for zero rows");

  result = md::editing::applyCommand(doc, md::editing::ApplyParagraphStyleCommand{{0, 0, 0, false}, "Missing"});
  require(!result.success && result.error == md::editing::CommandError::MissingReference,
          "apply paragraph style should fail for missing style");

  result = md::editing::applyCommand(doc, md::editing::ApplyNumberingCommand{{0, 0, 0, false}, 999, md::NumberingLevel::Level1});
  require(!result.success && result.error == md::editing::CommandError::MissingReference,
          "apply numbering should fail for missing definition");

  result = md::editing::applyCommand(doc, md::editing::InsertPictureCommand{{0, 0, 0, false}, 0, {}, md::FileType::PNG});
  require(!result.success && result.error == md::editing::CommandError::InvalidArgument,
          "insert picture should fail for empty image");
}

void testCommandDeterminism()
{
  auto buildAndApply = []() {
    md::Document doc;
    auto section = doc.addSection();
    auto paragraph = section->addParagraph();
    paragraph->addRichText("Base");

    auto result = md::editing::applyCommand(doc, md::editing::InsertParagraphCommand{0, 1});
    require(result.success, "determinism insert paragraph failed");

    result = md::editing::applyCommand(doc, md::editing::ReplaceParagraphTextCommand{{0, 1, 0, false}, "Tail"});
    require(result.success, "determinism replace paragraph failed");

    result = md::editing::applyCommand(doc, md::editing::InsertRichTextRunCommand{{0, 1, 0, false}, 1, " text"});
    require(result.success, "determinism insert run failed");

    return md::inspection::extractVisibleText(doc);
  };

  const auto a = buildAndApply();
  const auto b = buildAndApply();
  require(a == b, "same command sequence should produce deterministic text output");
}
}

int main()
{
  testCommandsSuccessFlow();
  testCommandFailures();
  testCommandDeterminism();
  return 0;
}

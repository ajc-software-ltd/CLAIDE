#pragma once

#include "config.hpp"
#include "inspection/semantic.hpp"
#include "utils/file.hpp"
#include "word/main/document.hpp"
#include "word/main/properties/paragraph.hpp"
#include "word/main/properties/richtext.hpp"
#include "word/main/properties/section.hpp"

#include <string>
#include <variant>

namespace MINIDOCX_NAMESPACE::editing
{
  enum class CommandError
  {
    None,
    InvalidPath,
    InvalidNodeType,
    InvalidArgument,
    MissingReference,
    OperationNotAllowed,
  };

  struct CommandResult
  {
    bool success = false;
    CommandError error = CommandError::None;
    std::string message;

    static CommandResult ok() { return {true, CommandError::None, {}}; }
    static CommandResult fail(CommandError code, std::string reason)
    {
      return {false, code, std::move(reason)};
    }
  };

  struct InsertSectionCommand
  {
    size_t index = 0;
  };

  struct DeleteSectionCommand
  {
    size_t index = 0;
  };

  struct InsertParagraphCommand
  {
    size_t sectionIndex = 0;
    size_t blockIndex = 0;
  };

  struct DeleteBlockCommand
  {
    size_t sectionIndex = 0;
    size_t blockIndex = 0;
  };

  struct ReplaceParagraphTextCommand
  {
    inspection::NodePath paragraphPath;
    std::string text;
  };

  struct InsertRichTextRunCommand
  {
    inspection::NodePath paragraphPath;
    size_t runIndex = 0;
    std::string text;
  };

  struct ReplaceRunTextCommand
  {
    inspection::NodePath runPath;
    std::string text;
  };

  struct InsertPictureCommand
  {
    inspection::NodePath paragraphPath;
    size_t runIndex = 0;
    Buffer image;
    FileType fileType = FileType::Unknown;
  };

  struct CreateTableCommand
  {
    size_t sectionIndex = 0;
    size_t blockIndex = 0;
    size_t rows = 0;
    size_t cols = 0;
  };

  struct MergeCellsCommand
  {
    inspection::NodePath tablePath;
    size_t row = 0;
    size_t col = 0;
    size_t rows = 0;
    size_t cols = 0;
  };

  struct SplitCellCommand
  {
    inspection::NodePath tablePath;
    size_t row = 0;
    size_t col = 0;
  };

  struct ApplyParagraphStyleCommand
  {
    inspection::NodePath paragraphPath;
    std::string styleId;
  };

  struct ApplyCharacterStyleCommand
  {
    inspection::NodePath runPath;
    std::string styleId;
  };

  struct SetParagraphPropertiesCommand
  {
    inspection::NodePath paragraphPath;
    ParagraphProperties properties;
  };

  struct SetRunPropertiesCommand
  {
    inspection::NodePath runPath;
    RichTextProperties properties;
  };

  struct ApplyNumberingCommand
  {
    inspection::NodePath paragraphPath;
    NumberingId numberingId = 0;
    NumberingLevel level = NumberingLevel::Level1;
  };

  struct UpdateSectionPropertiesCommand
  {
    size_t sectionIndex = 0;
    SectionProperties properties;
  };

  using EditCommand = std::variant<
      InsertSectionCommand,
      DeleteSectionCommand,
      InsertParagraphCommand,
      DeleteBlockCommand,
      ReplaceParagraphTextCommand,
      InsertRichTextRunCommand,
      ReplaceRunTextCommand,
      InsertPictureCommand,
      CreateTableCommand,
      MergeCellsCommand,
      SplitCellCommand,
      ApplyParagraphStyleCommand,
      ApplyCharacterStyleCommand,
      SetParagraphPropertiesCommand,
      SetRunPropertiesCommand,
      ApplyNumberingCommand,
      UpdateSectionPropertiesCommand>;

  MINIDOCX_API CommandResult applyCommand(Document& document, const EditCommand& command);
}

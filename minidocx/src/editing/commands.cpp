#include "editing/commands.hpp"

#include "utils/exceptions.hpp"
#include "word/main/paragraph.hpp"
#include "word/main/picture.hpp"
#include "word/main/richtext.hpp"
#include "word/main/section.hpp"
#include "word/main/table.hpp"

#include <algorithm>

namespace MINIDOCX_NAMESPACE::editing
{
  namespace
  {
    template <typename T>
    CommandResult executeCommand(Document& document, const T& command);

    BlockPointer requireBlock(const SectionPointer& section, const size_t blockIndex, const char* context)
    {
      if (section == nullptr || blockIndex >= section->blockCount())
        throw std::runtime_error(std::string(context) + ": block path is out of range");
      return section->blockAt(blockIndex);
    }

    ParagraphPointer requireParagraph(const SectionPointer& section, const inspection::NodePath& path, const char* context)
    {
      auto block = requireBlock(section, path.blockIndex, context);
      if (block->type() != BlockType::Paragraph)
        throw std::runtime_error(std::string(context) + ": target block is not a paragraph");
      return std::dynamic_pointer_cast<Paragraph>(block);
    }

    TablePointer requireTable(const SectionPointer& section, const inspection::NodePath& path, const char* context)
    {
      auto block = requireBlock(section, path.blockIndex, context);
      if (block->type() != BlockType::Table)
        throw std::runtime_error(std::string(context) + ": target block is not a table");
      return std::dynamic_pointer_cast<Table>(block);
    }

    RichTextPointer requireRichTextRun(const ParagraphPointer& paragraph, const inspection::NodePath& path, const char* context)
    {
      if (!path.hasRunIndex || path.runIndex >= paragraph->runCount())
        throw std::runtime_error(std::string(context) + ": run path is out of range");

      const auto run = paragraph->runAt(path.runIndex);
      if (run->type() != RunType::RichText)
        throw std::runtime_error(std::string(context) + ": target run is not rich text");
      return std::dynamic_pointer_cast<RichText>(run);
    }

    bool hasParagraphStyle(const Document& document, const std::string& styleId)
    {
      std::string normalized = styleId;
      normalized.erase(std::remove(normalized.begin(), normalized.end(), ' '), normalized.end());
      const auto& styles = document.paragraphStyles();
      return styles.find(normalized) != styles.end();
    }

    bool hasCharacterStyle(const Document& document, const std::string& styleId)
    {
      std::string normalized = styleId;
      normalized.erase(std::remove(normalized.begin(), normalized.end(), ' '), normalized.end());
      const auto& styles = document.characterStyles();
      return styles.find(normalized) != styles.end();
    }

    bool hasNumberingDefinition(const Document& document, const NumberingId id)
    {
      return document.numberingDefinitions().find(id) != document.numberingDefinitions().end();
    }

    template <typename T>
    CommandResult executeCommand(Document& document, const T& command)
    {
      (void)document;
      (void)command;
      return CommandResult::fail(CommandError::OperationNotAllowed, "unsupported command type");
    }

    template <>
    CommandResult executeCommand(Document& document, const InsertSectionCommand& command)
    {
      if (command.index > document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      document.insertSection(command.index);
      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const DeleteSectionCommand& command)
    {
      if (command.index >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (document.sectionCount() == 1)
        return CommandResult::fail(CommandError::OperationNotAllowed, "document must contain at least one section");

      document.deleteSectionAt(command.index);
      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const InsertParagraphCommand& command)
    {
      if (command.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      const auto section = document.sectionAt(command.sectionIndex);
      if (command.blockIndex > section->blockCount())
        return CommandResult::fail(CommandError::InvalidPath, "block index is out of range");

      section->insertParagraph(command.blockIndex);
      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const DeleteBlockCommand& command)
    {
      if (command.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      const auto section = document.sectionAt(command.sectionIndex);
      if (!section->deleteBlockAt(command.blockIndex))
        return CommandResult::fail(CommandError::InvalidPath, "block index is out of range");

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const ReplaceParagraphTextCommand& command)
    {
      if (command.paragraphPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      try {
        const auto section = document.sectionAt(command.paragraphPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.paragraphPath, "replace paragraph text");
        paragraph->clear();
        paragraph->addRichText(command.text);
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const InsertRichTextRunCommand& command)
    {
      if (command.paragraphPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      try {
        const auto section = document.sectionAt(command.paragraphPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.paragraphPath, "insert rich text run");
        if (command.runIndex > paragraph->runCount())
          return CommandResult::fail(CommandError::InvalidPath, "run index is out of range");
        paragraph->insertRichText(command.runIndex, command.text);
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const ReplaceRunTextCommand& command)
    {
      if (command.runPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      try {
        const auto section = document.sectionAt(command.runPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.runPath, "replace run text");
        const auto run = requireRichTextRun(paragraph, command.runPath, "replace run text");
        run->setText(command.text);
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const InsertPictureCommand& command)
    {
      if (command.paragraphPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (command.image.empty())
        return CommandResult::fail(CommandError::InvalidArgument, "image buffer is empty");
      if (command.fileType == FileType::Unknown)
        return CommandResult::fail(CommandError::InvalidArgument, "image file type is unknown");

      try {
        const auto section = document.sectionAt(command.paragraphPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.paragraphPath, "insert picture");
        if (command.runIndex > paragraph->runCount())
          return CommandResult::fail(CommandError::InvalidPath, "run index is out of range");

        const auto relId = document.addImage(command.image, command.fileType);
        paragraph->insertPicture(command.runIndex, relId);
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const CreateTableCommand& command)
    {
      if (command.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (command.rows == 0 || command.cols == 0)
        return CommandResult::fail(CommandError::InvalidArgument, "table rows and cols must be greater than zero");

      const auto section = document.sectionAt(command.sectionIndex);
      if (command.blockIndex > section->blockCount())
        return CommandResult::fail(CommandError::InvalidPath, "block index is out of range");

      section->insertTable(command.blockIndex, command.rows, command.cols);
      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const MergeCellsCommand& command)
    {
      if (command.tablePath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (command.rows == 0 || command.cols == 0)
        return CommandResult::fail(CommandError::InvalidArgument, "merge rows and cols must be greater than zero");

      try {
        const auto section = document.sectionAt(command.tablePath.sectionIndex);
        const auto table = requireTable(section, command.tablePath, "merge cells");
        table->merge(command.row, command.col, command.rows, command.cols);
      }
      catch (const invalid_parameter&) {
        return CommandResult::fail(CommandError::InvalidArgument, "invalid merge range");
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const SplitCellCommand& command)
    {
      if (command.tablePath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      try {
        const auto section = document.sectionAt(command.tablePath.sectionIndex);
        const auto table = requireTable(section, command.tablePath, "split cell");
        table->split(command.row, command.col);
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const ApplyParagraphStyleCommand& command)
    {
      if (command.paragraphPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (!hasParagraphStyle(document, command.styleId))
        return CommandResult::fail(CommandError::MissingReference, "paragraph style does not exist");

      try {
        const auto section = document.sectionAt(command.paragraphPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.paragraphPath, "apply paragraph style");
        paragraph->prop_.style_ = command.styleId;
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const ApplyCharacterStyleCommand& command)
    {
      if (command.runPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (!hasCharacterStyle(document, command.styleId))
        return CommandResult::fail(CommandError::MissingReference, "character style does not exist");

      try {
        const auto section = document.sectionAt(command.runPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.runPath, "apply character style");
        const auto run = requireRichTextRun(paragraph, command.runPath, "apply character style");
        run->prop_.style_ = command.styleId;
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const SetParagraphPropertiesCommand& command)
    {
      if (command.paragraphPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      try {
        const auto section = document.sectionAt(command.paragraphPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.paragraphPath, "set paragraph properties");
        paragraph->prop_ = command.properties;
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const SetRunPropertiesCommand& command)
    {
      if (command.runPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      try {
        const auto section = document.sectionAt(command.runPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.runPath, "set run properties");
        const auto run = requireRichTextRun(paragraph, command.runPath, "set run properties");
        run->prop_ = command.properties;
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const ApplyNumberingCommand& command)
    {
      if (command.paragraphPath.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");
      if (!hasNumberingDefinition(document, command.numberingId))
        return CommandResult::fail(CommandError::MissingReference, "numbering definition does not exist");

      try {
        const auto section = document.sectionAt(command.paragraphPath.sectionIndex);
        const auto paragraph = requireParagraph(section, command.paragraphPath, "apply numbering");
        paragraph->numId_ = command.numberingId;
        paragraph->level_ = command.level;
      }
      catch (const std::runtime_error& ex) {
        return CommandResult::fail(CommandError::InvalidNodeType, ex.what());
      }

      return CommandResult::ok();
    }

    template <>
    CommandResult executeCommand(Document& document, const UpdateSectionPropertiesCommand& command)
    {
      if (command.sectionIndex >= document.sectionCount())
        return CommandResult::fail(CommandError::InvalidPath, "section index is out of range");

      const auto section = document.sectionAt(command.sectionIndex);
      section->prop_ = command.properties;
      return CommandResult::ok();
    }
  }

  CommandResult applyCommand(Document& document, const EditCommand& command)
  {
    try {
      return std::visit(
          [&](const auto& cmd) {
            return executeCommand(document, cmd);
          },
          command);
    }
    catch (const invalid_parameter& ex) {
      return CommandResult::fail(CommandError::InvalidArgument, ex.what());
    }
    catch (const std::exception& ex) {
      return CommandResult::fail(CommandError::OperationNotAllowed, ex.what());
    }
  }
}

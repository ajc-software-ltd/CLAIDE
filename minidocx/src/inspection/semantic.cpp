#include "inspection/semantic.hpp"

#include <set>
#include <sstream>

namespace MINIDOCX_NAMESPACE::inspection
{
  namespace
  {
    std::string runText(const RunPointer& run)
    {
      if (run->type() != RunType::RichText)
        return {};
      return std::dynamic_pointer_cast<RichText>(run)->text();
    }

    std::string paragraphText(const ParagraphPointer& paragraph)
    {
      std::string out;
      for (const auto& run : paragraph->runs()) {
        out += runText(run);
      }
      return out;
    }

    std::string cellText(const CellPointer& cell)
    {
      std::string out;
      bool first = true;
      for (const auto& block : cell->blocks()) {
        if (block->type() != BlockType::Paragraph)
          continue;
        const auto para = std::dynamic_pointer_cast<Paragraph>(block);
        if (!first)
          out += '\n';
        out += paragraphText(para);
        first = false;
      }
      return out;
    }

  }

  DocumentStats summarize(const Document& document)
  {
    DocumentStats stats;
    const auto sections = document.sections();
    stats.sectionCount = sections.size();

    for (const auto& section : sections) {
      const auto blocks = section->blocks();
      stats.blockCount += blocks.size();
      for (const auto& block : blocks) {
        if (block->type() == BlockType::Paragraph) {
          stats.paragraphCount++;
          const auto para = std::dynamic_pointer_cast<Paragraph>(block);
          const auto runs = para->runs();
          stats.runCount += runs.size();
          for (const auto& run : runs) {
            if (run->type() == RunType::Picture)
              stats.pictureCount++;
          }
        }
        else if (block->type() == BlockType::Table) {
          stats.tableCount++;
          const auto table = std::dynamic_pointer_cast<Table>(block);
          const auto rect = table->rect();
          std::set<const Cell*> seen;
          for (size_t row = 0; row < rect.rows(); row++) {
            for (size_t col = 0; col < rect.cols(); col++) {
              const auto cell = table->cellAt(row, col);
              if (!seen.insert(cell.get()).second)
                continue;
              stats.cellCount++;
            }
          }
        }
      }
    }

    return stats;
  }

  std::vector<SectionInfo> listSections(const Document& document)
  {
    std::vector<SectionInfo> out;
    const auto sections = document.sections();
    out.reserve(sections.size());

    size_t sectionIndex = 0;
    for (const auto& section : sections) {
      SectionInfo info;
      info.sectionIndex = sectionIndex++;
      const auto blocks = section->blocks();
      info.blockCount = blocks.size();
      for (const auto& block : blocks) {
        if (block->type() == BlockType::Paragraph)
          info.paragraphCount++;
        else if (block->type() == BlockType::Table)
          info.tableCount++;
      }
      out.push_back(info);
    }

    return out;
  }

  std::vector<ParagraphInfo> listParagraphs(const Document& document)
  {
    std::vector<ParagraphInfo> out;
    const auto sections = document.sections();

    size_t sectionIndex = 0;
    for (const auto& section : sections) {
      const auto blocks = section->blocks();
      size_t blockIndex = 0;
      for (const auto& block : blocks) {
        if (block->type() == BlockType::Paragraph) {
          const auto paragraph = std::dynamic_pointer_cast<Paragraph>(block);
          ParagraphInfo info;
          info.path = {.sectionIndex = sectionIndex, .blockIndex = blockIndex};
          info.style = paragraph->prop_.style_;
          info.numberingId = paragraph->numId_;
          info.numberingLevel = paragraph->level_;
          info.outlineLevel = paragraph->prop_.outlineLevel_;
          info.visibleText = paragraphText(paragraph);
          out.push_back(std::move(info));
        }
        blockIndex++;
      }
      sectionIndex++;
    }

    return out;
  }

  std::vector<ParagraphInfo> listOutlineParagraphs(const Document& document)
  {
    std::vector<ParagraphInfo> out;
    for (auto info : listParagraphs(document)) {
      const bool outline = info.outlineLevel != ParagraphProperties::OutlineLevel::BodyText;
      if (outline)
        out.push_back(std::move(info));
    }
    return out;
  }

  std::vector<RunInfo> listRuns(const Document& document)
  {
    std::vector<RunInfo> out;
    const auto sections = document.sections();

    size_t sectionIndex = 0;
    for (const auto& section : sections) {
      const auto blocks = section->blocks();
      size_t blockIndex = 0;
      for (const auto& block : blocks) {
        if (block->type() != BlockType::Paragraph) {
          blockIndex++;
          continue;
        }

        const auto paragraph = std::dynamic_pointer_cast<Paragraph>(block);
        const auto runs = paragraph->runs();
        size_t runIndex = 0;
        for (const auto& run : runs) {
          RunInfo info;
          info.path = {.sectionIndex = sectionIndex, .blockIndex = blockIndex, .runIndex = runIndex, .hasRunIndex = true};
          info.runType = run->type();

          if (run->type() == RunType::RichText) {
            const auto rich = std::dynamic_pointer_cast<RichText>(run);
            info.visibleText = rich->text();
            info.style = rich->prop_.style_;
          }
          else if (run->type() == RunType::Picture) {
            const auto picture = std::dynamic_pointer_cast<Picture>(run);
            info.imageRelationshipId = picture->id_;
            info.imageWidth = picture->prop_.extent_.width_;
            info.imageHeight = picture->prop_.extent_.height_;
          }

          out.push_back(std::move(info));
          runIndex++;
        }

        blockIndex++;
      }
      sectionIndex++;
    }

    return out;
  }

  std::vector<TableInfo> listTables(const Document& document)
  {
    std::vector<TableInfo> out;
    const auto sections = document.sections();

    size_t sectionIndex = 0;
    for (const auto& section : sections) {
      const auto blocks = section->blocks();
      size_t blockIndex = 0;
      for (const auto& block : blocks) {
        if (block->type() == BlockType::Table) {
          const auto table = std::dynamic_pointer_cast<Table>(block);
          const auto rect = table->rect();
          TableInfo info;
          info.path.sectionIndex = sectionIndex;
          info.path.blockIndex = blockIndex;
          info.rows = rect.rows();
          info.cols = rect.cols();
          out.push_back(std::move(info));
        }
        blockIndex++;
      }
      sectionIndex++;
    }

    return out;
  }

  std::vector<CellInfo> listCells(const Document& document)
  {
    std::vector<CellInfo> out;
    const auto sections = document.sections();

    size_t sectionIndex = 0;
    for (const auto& section : sections) {
      const auto blocks = section->blocks();
      size_t blockIndex = 0;
      for (const auto& block : blocks) {
        if (block->type() != BlockType::Table) {
          blockIndex++;
          continue;
        }

        const auto table = std::dynamic_pointer_cast<Table>(block);
        const auto rect = table->rect();
        std::set<const Cell*> seen;

        for (size_t row = 0; row < rect.rows(); row++) {
          for (size_t col = 0; col < rect.cols(); col++) {
            const auto cell = table->cellAt(row, col);
            if (!seen.insert(cell.get()).second)
              continue;

            CellInfo info;
            info.path.sectionIndex = sectionIndex;
            info.path.blockIndex = blockIndex;
            info.path.row = cell->rect().row();
            info.path.col = cell->rect().col();
            info.rowSpan = cell->rect().rows();
            info.colSpan = cell->rect().cols();
            info.visibleText = cellText(cell);
            out.push_back(std::move(info));
          }
        }

        blockIndex++;
      }
      sectionIndex++;
    }

    return out;
  }

  std::vector<PictureInfo> listPictures(const Document& document)
  {
    std::vector<PictureInfo> out;
    for (const auto& run : listRuns(document)) {
      if (run.runType != RunType::Picture)
        continue;
      PictureInfo info;
      info.path = run.path;
      info.relationshipId = run.imageRelationshipId;
      info.width = run.imageWidth;
      info.height = run.imageHeight;
      out.push_back(std::move(info));
    }
    return out;
  }

  bool hasNumbering(const Document& document)
  {
    for (const auto& paragraph : listParagraphs(document)) {
      if (paragraph.numberingId > 0)
        return true;
    }
    return false;
  }

  std::string extractVisibleText(const Document& document)
  {
    std::ostringstream out;
    bool first = true;
    for (const auto& paragraph : listParagraphs(document)) {
      if (!first)
        out << '\n';
      out << paragraph.visibleText;
      first = false;
    }

    for (const auto& cell : listCells(document)) {
      if (cell.visibleText.empty())
        continue;
      if (!first)
        out << '\n';
      out << cell.visibleText;
      first = false;
    }

    return out.str();
  }

  std::string extractSectionVisibleText(const Document& document, size_t sectionIndex)
  {
    std::ostringstream out;
    bool first = true;

    for (const auto& paragraph : listParagraphs(document)) {
      if (paragraph.path.sectionIndex != sectionIndex)
        continue;
      if (!first)
        out << '\n';
      out << paragraph.visibleText;
      first = false;
    }

    for (const auto& cell : listCells(document)) {
      if (cell.path.sectionIndex != sectionIndex || cell.visibleText.empty())
        continue;
      if (!first)
        out << '\n';
      out << cell.visibleText;
      first = false;
    }

    return out.str();
  }
}

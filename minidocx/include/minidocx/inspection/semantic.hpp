#pragma once

#include "word/main/document.hpp"
#include "word/main/section.hpp"
#include "word/main/paragraph.hpp"
#include "word/main/richtext.hpp"
#include "word/main/picture.hpp"
#include "word/main/table.hpp"
#include "word/main/cell.hpp"

#include <string>
#include <vector>

namespace MINIDOCX_NAMESPACE::inspection
{
  struct NodePath
  {
    size_t sectionIndex = 0;
    size_t blockIndex = 0;
    size_t runIndex = 0;
    bool hasRunIndex = false;
  };

  struct CellPath
  {
    size_t sectionIndex = 0;
    size_t blockIndex = 0;
    size_t row = 0;
    size_t col = 0;
  };

  struct DocumentStats
  {
    size_t sectionCount = 0;
    size_t blockCount = 0;
    size_t paragraphCount = 0;
    size_t runCount = 0;
    size_t tableCount = 0;
    size_t cellCount = 0;
    size_t pictureCount = 0;
  };

  struct SectionInfo
  {
    size_t sectionIndex = 0;
    size_t blockCount = 0;
    size_t paragraphCount = 0;
    size_t tableCount = 0;
  };

  struct ParagraphInfo
  {
    NodePath path;
    std::string style;
    NumberingId numberingId = 0;
    NumberingLevel numberingLevel = NumberingLevel::Level1;
    ParagraphProperties::OutlineLevel outlineLevel = ParagraphProperties::OutlineLevel::BodyText;
    std::string visibleText;
  };

  struct RunInfo
  {
    NodePath path;
    RunType runType = RunType::RichText;
    std::string visibleText;
    std::string style;
    RelationshipId imageRelationshipId = 0;
    size_t imageWidth = 0;
    size_t imageHeight = 0;
  };

  struct TableInfo
  {
    NodePath path;
    size_t rows = 0;
    size_t cols = 0;
  };

  struct CellInfo
  {
    CellPath path;
    size_t rowSpan = 1;
    size_t colSpan = 1;
    std::string visibleText;
  };

  struct PictureInfo
  {
    NodePath path;
    RelationshipId relationshipId = 0;
    size_t width = 0;
    size_t height = 0;
  };

  DocumentStats summarize(const Document& document);

  std::vector<SectionInfo> listSections(const Document& document);
  std::vector<ParagraphInfo> listParagraphs(const Document& document);
  std::vector<ParagraphInfo> listOutlineParagraphs(const Document& document);
  std::vector<RunInfo> listRuns(const Document& document);
  std::vector<TableInfo> listTables(const Document& document);
  std::vector<CellInfo> listCells(const Document& document);
  std::vector<PictureInfo> listPictures(const Document& document);

  bool hasNumbering(const Document& document);

  std::string extractVisibleText(const Document& document);
  std::string extractSectionVisibleText(const Document& document, size_t sectionIndex);
}

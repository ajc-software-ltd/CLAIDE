#include "inspection/layout.hpp"

#include "inspection/style_resolution.hpp"
#include "word/main/cell.hpp"
#include "word/main/document.hpp"
#include "word/main/paragraph.hpp"
#include "word/main/picture.hpp"
#include "word/main/richtext.hpp"
#include "word/main/section.hpp"
#include "word/main/table.hpp"

#include <algorithm>
#include <set>
#include <sstream>

namespace MINIDOCX_NAMESPACE::inspection
{
  namespace
  {
    constexpr size_t k_DefaultLineHeightTwips = 240;
    constexpr size_t k_DefaultRowHeightTwips = 360;
    constexpr size_t k_MinCharWidthTwips = 60;
    constexpr size_t k_EmuPerTwip = 635;

    size_t clampNonZero(const size_t value, const size_t fallback)
    {
      return value == 0 ? fallback : value;
    }

    size_t toTwipsFromEmu(const size_t emu)
    {
      return emu / k_EmuPerTwip;
    }

    size_t paragraphSpacingBefore(const ParagraphProperties& prop, const size_t lineHeight)
    {
      if (!prop.spacing_.has_value())
        return 0;

      const auto before = prop.spacing_->before_;
      if (before.type_ == ParagraphProperties::SpacingType::Absolute)
        return before.value_;
      if (before.type_ == ParagraphProperties::SpacingType::Lines)
        return before.value_ * lineHeight / 100;
      return 0;
    }

    size_t paragraphSpacingAfter(const ParagraphProperties& prop, const size_t lineHeight)
    {
      if (!prop.spacing_.has_value())
        return 0;

      const auto after = prop.spacing_->after_;
      if (after.type_ == ParagraphProperties::SpacingType::Absolute)
        return after.value_;
      if (after.type_ == ParagraphProperties::SpacingType::Lines)
        return after.value_ * lineHeight / 100;
      return 0;
    }

    size_t lineHeightFor(const RichTextProperties& prop)
    {
      return prop.fontSize_ == 0 ? k_DefaultLineHeightTwips : prop.fontSize_ * 10;
    }

    size_t charWidthFor(const RichTextProperties& prop)
    {
      return prop.fontSize_ == 0 ? k_MinCharWidthTwips : std::max(k_MinCharWidthTwips, prop.fontSize_ * 5);
    }

    std::vector<std::string> splitLines(const std::string& text, const size_t maxWidth, const size_t charWidth)
    {
      const size_t width = std::max<size_t>(1, maxWidth);
      const size_t widthPerChar = std::max<size_t>(1, charWidth);
      const size_t maxChars = std::max<size_t>(1, width / widthPerChar);

      std::vector<std::string> lines;
      std::string current;
      current.reserve(maxChars);

      for (char ch : text) {
        if (ch == '\r')
          continue;
        if (ch == '\t')
          ch = ' ';
        if (ch == '\n') {
          lines.push_back(current);
          current.clear();
          continue;
        }

        if (current.size() >= maxChars) {
          lines.push_back(current);
          current.clear();
        }
        current.push_back(ch);
      }

      lines.push_back(current);
      return lines;
    }

    struct SectionGeometry
    {
      size_t width = A4_W;
      size_t height = A4_H;
      LayoutRect content;
    };

    SectionGeometry sectionGeometry(const Section& section)
    {
      SectionGeometry geometry;
      geometry.width = section.prop_.size_.width_;
      geometry.height = section.prop_.size_.height_;
      if (section.prop_.landscape_)
        std::swap(geometry.width, geometry.height);

      const size_t left = section.prop_.margins_.left_;
      const size_t top = section.prop_.margins_.top_;
      const size_t right = section.prop_.margins_.right_;
      const size_t bottom = section.prop_.margins_.bottom_;

      geometry.content.x = left;
      geometry.content.y = top;
      geometry.content.width = geometry.width > left + right ? geometry.width - left - right : 1;
      geometry.content.height = geometry.height > top + bottom ? geometry.height - top - bottom : 1;
      return geometry;
    }

    PageLayout& ensurePage(DocumentLayout& layout, const size_t sectionIndex, const SectionGeometry& geometry)
    {
      PageLayout page;
      page.pageIndex = layout.pages.size();
      page.sectionIndex = sectionIndex;
      page.pageRect = {0, 0, geometry.width, geometry.height};
      page.contentRect = geometry.content;
      layout.pages.push_back(page);
      return layout.pages.back();
    }

    void addNodeRef(
        DocumentLayout& layout,
        const LayoutNodeKind kind,
        const size_t pageIndex,
        const LayoutRect rect,
        const NodePath* path,
        const CellPath* cellPath)
    {
      LayoutNodeRef node;
      node.kind = kind;
      node.pageIndex = pageIndex;
      node.rect = rect;
      if (path != nullptr) {
        node.path = *path;
        node.hasNodePath = true;
      }
      if (cellPath != nullptr) {
        node.cellPath = *cellPath;
        node.hasCellPath = true;
      }
      layout.nodes.push_back(node);
    }
  }

  DocumentLayout buildLayout(const Document& document)
  {
    DocumentLayout layout;
    const auto sections = document.sections();

    size_t sectionIndex = 0;
    for (const auto& section : sections) {
      const SectionGeometry geometry = sectionGeometry(*section);
      PageLayout* page = &ensurePage(layout, sectionIndex, geometry);
      size_t cursorY = page->contentRect.y;

      const auto blocks = section->blocks();
      size_t blockIndex = 0;
      for (const auto& block : blocks) {
        const NodePath blockPath{sectionIndex, blockIndex, 0, false};
        const size_t contentBottom = page->contentRect.y + page->contentRect.height;

        if (block->type() == BlockType::Paragraph) {
          const auto paragraph = std::dynamic_pointer_cast<Paragraph>(block);
          const auto paragraphResolved = resolveParagraphFormatting(document, *paragraph);
          const size_t paragraphLineHeight = lineHeightFor(paragraphResolved.runDefaults);
          const size_t paragraphBefore = paragraphSpacingBefore(paragraphResolved.paragraph, paragraphLineHeight);
          const size_t paragraphAfter = paragraphSpacingAfter(paragraphResolved.paragraph, paragraphLineHeight);
          const size_t paragraphStartY = cursorY + paragraphBefore;

          if (paragraphStartY + paragraphLineHeight > contentBottom) {
            page = &ensurePage(layout, sectionIndex, geometry);
            cursorY = page->contentRect.y;
          }

          size_t paragraphTop = cursorY + paragraphBefore;
          size_t paragraphBottom = paragraphTop;
          size_t runIndex = 0;
          for (const auto& run : paragraph->runs()) {
            NodePath runPath{sectionIndex, blockIndex, runIndex, true};

            if (run->type() == RunType::RichText) {
              const auto rich = std::dynamic_pointer_cast<RichText>(run);
              const auto runResolved = resolveRunFormatting(document, *paragraph, *rich);
              const size_t lineHeight = lineHeightFor(runResolved.run);
              const size_t charWidth = charWidthFor(runResolved.run);
              const auto lines = splitLines(rich->text(), page->contentRect.width, charWidth);

              size_t runTop = paragraphBottom;
              size_t runBottom = runTop;
              for (const auto& lineText : lines) {
                if (paragraphBottom + lineHeight > page->contentRect.y + page->contentRect.height) {
                  page = &ensurePage(layout, sectionIndex, geometry);
                  cursorY = page->contentRect.y;
                  paragraphBottom = cursorY;
                }

                const size_t lineWidth = std::min(page->contentRect.width, lineText.size() * charWidth);
                LayoutRect lineRect{page->contentRect.x, paragraphBottom, lineWidth, lineHeight};

                LineLayout line;
                line.paragraphPath = blockPath;
                line.lineIndex = page->lines.size();
                line.text = lineText;
                line.rect = lineRect;
                LineRunLayout lineRun;
                lineRun.path = runPath;
                lineRun.text = lineText;
                lineRun.rect = lineRect;
                line.runs.push_back(lineRun);
                page->lines.push_back(line);

                addNodeRef(layout, LayoutNodeKind::Line, page->pageIndex, lineRect, &blockPath, nullptr);
                paragraphBottom += lineHeight;
                runBottom = paragraphBottom;
              }

              if (!lines.empty()) {
                const LayoutRect runRect{page->contentRect.x, runTop, page->contentRect.width, runBottom - runTop};
                addNodeRef(layout, LayoutNodeKind::Run, page->pageIndex, runRect, &runPath, nullptr);
              }
            }
            else if (run->type() == RunType::Picture) {
              const auto picture = std::dynamic_pointer_cast<Picture>(run);
              const size_t width = std::min(page->contentRect.width, clampNonZero(toTwipsFromEmu(picture->prop_.extent_.width_), 1));
              const size_t height = clampNonZero(toTwipsFromEmu(picture->prop_.extent_.height_), 1);

              if (paragraphBottom + height > page->contentRect.y + page->contentRect.height) {
                page = &ensurePage(layout, sectionIndex, geometry);
                cursorY = page->contentRect.y;
                paragraphBottom = cursorY;
              }

              const LayoutRect pictureRect{page->contentRect.x, paragraphBottom, width, height};
              addNodeRef(layout, LayoutNodeKind::Picture, page->pageIndex, pictureRect, &runPath, nullptr);
              paragraphBottom += height;
            }

            runIndex++;
          }

          if (paragraphBottom < paragraphTop)
            paragraphBottom = paragraphTop + paragraphLineHeight;
          paragraphBottom += paragraphAfter;
          const LayoutRect paragraphRect{
              page->contentRect.x,
              paragraphTop,
              page->contentRect.width,
              paragraphBottom - paragraphTop,
          };
          addNodeRef(layout, LayoutNodeKind::Paragraph, page->pageIndex, paragraphRect, &blockPath, nullptr);
          cursorY = paragraphBottom;
        }
        else if (block->type() == BlockType::Table) {
          const auto table = std::dynamic_pointer_cast<Table>(block);
          const Rect rect = table->rect();
          const size_t cols = std::max<size_t>(1, rect.cols());
          const size_t rows = std::max<size_t>(1, rect.rows());

          size_t tableWidth = page->contentRect.width;
          if (table->prop_.width_.type_ == TableProperties::WidthType::Absolute)
            tableWidth = std::min(page->contentRect.width, table->prop_.width_.value_);
          else if (table->prop_.width_.type_ == TableProperties::WidthType::Percent)
            tableWidth = std::min(page->contentRect.width, page->contentRect.width * table->prop_.width_.value_ / 5000);

          const size_t colWidth = std::max<size_t>(1, tableWidth / cols);
          const size_t tableHeight = rows * k_DefaultRowHeightTwips;
          if (cursorY + tableHeight > contentBottom) {
            page = &ensurePage(layout, sectionIndex, geometry);
            cursorY = page->contentRect.y;
          }

          const LayoutRect tableRect{page->contentRect.x, cursorY, tableWidth, tableHeight};
          addNodeRef(layout, LayoutNodeKind::Table, page->pageIndex, tableRect, &blockPath, nullptr);

          std::set<const Cell*> seen;
          for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
              const auto cell = table->cellAt(row, col);
              if (!seen.insert(cell.get()).second)
                continue;

              const Rect cellRect = cell->rect();
              const LayoutRect cellLayoutRect{
                  page->contentRect.x + cellRect.col() * colWidth,
                  cursorY + cellRect.row() * k_DefaultRowHeightTwips,
                  std::max<size_t>(1, cellRect.cols() * colWidth),
                  std::max<size_t>(1, cellRect.rows() * k_DefaultRowHeightTwips),
              };
              const CellPath cellPath{sectionIndex, blockIndex, cellRect.row(), cellRect.col()};
              addNodeRef(layout, LayoutNodeKind::Cell, page->pageIndex, cellLayoutRect, nullptr, &cellPath);
            }
          }

          cursorY += tableHeight;
        }

        blockIndex++;
      }

      sectionIndex++;
    }

    return layout;
  }
}

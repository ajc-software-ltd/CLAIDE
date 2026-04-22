#pragma once

// Public neutral layout API.
// Builds renderer-independent page/line/node geometry from document state.

#include "config.hpp"
#include "inspection/semantic.hpp"

#include <string>
#include <vector>

namespace MINIDOCX_NAMESPACE::inspection
{
  struct LayoutRect
  {
    size_t x = 0;
    size_t y = 0;
    size_t width = 0;
    size_t height = 0;
  };

  enum class LayoutNodeKind
  {
    Paragraph,
    Line,
    Run,
    Table,
    Cell,
    Picture,
  };

  struct LineRunLayout
  {
    NodePath path;
    std::string text;
    LayoutRect rect;
  };

  struct LineLayout
  {
    NodePath paragraphPath;
    size_t lineIndex = 0;
    std::string text;
    LayoutRect rect;
    std::vector<LineRunLayout> runs;
  };

  struct LayoutNodeRef
  {
    LayoutNodeKind kind = LayoutNodeKind::Paragraph;
    size_t pageIndex = 0;
    NodePath path;
    bool hasNodePath = false;
    CellPath cellPath;
    bool hasCellPath = false;
    LayoutRect rect;
  };

  struct PageLayout
  {
    size_t pageIndex = 0;
    size_t sectionIndex = 0;
    LayoutRect pageRect;
    LayoutRect contentRect;
    std::vector<LineLayout> lines;
  };

  struct DocumentLayout
  {
    std::vector<PageLayout> pages;
    std::vector<LayoutNodeRef> nodes;
  };

  MINIDOCX_API DocumentLayout buildLayout(const Document& document);
}

// ============================================================================
// MINIDOCX
// ============================================================================
// File:        container.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "config.hpp"
#include "word/main/base.hpp"

#include <memory>
#include <list>


namespace MINIDOCX_NAMESPACE
{
  class Paragraph;
  class ParagraphProperties;
  class Table;
  using BlockPointer = std::shared_ptr<Block>;
  using ParagraphPointer = std::shared_ptr<Paragraph>;
  using TablePointer = std::shared_ptr<Table>;


  class MINIDOCX_API Container : public Destroyable
  {
  private:
    std::list<BlockPointer> blocks_;

  public:
    inline std::list<BlockPointer> blocks() const { return blocks_; }

    ParagraphPointer addParagraph();
    ParagraphPointer addParagraph(ParagraphProperties prop);
    
    TablePointer addTable(const size_t rows, const size_t cols);
    
    void deleteBlock(const BlockPointer& block);

  public:
    void clear() override;
  };

}

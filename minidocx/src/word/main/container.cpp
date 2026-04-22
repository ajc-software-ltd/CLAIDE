// ============================================================================
// MINIDOCX
// ============================================================================
// File:        container.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#include "word/main/container.hpp"
#include "word/main/paragraph.hpp"
#include "word/main/table.hpp"
#include "utils/exceptions.hpp"


namespace MINIDOCX_NAMESPACE
{
  BlockPointer Container::blockAt(const size_t index) const
  {
    if (index >= blocks_.size())
      throw invalid_parameter();
    return *std::next(blocks_.begin(), static_cast<std::ptrdiff_t>(index));
  }

  ParagraphPointer Container::addParagraph()
  {
    auto block{ std::make_shared<Paragraph>() };
    blocks_.push_back(block);
    return block;
  }

  ParagraphPointer Container::addParagraph(ParagraphProperties prop)
  {
    auto block{ addParagraph() };
    block->prop_ = std::move(prop);
    return block;
  }

  ParagraphPointer Container::insertParagraph(const size_t index)
  {
    if (index > blocks_.size())
      throw invalid_parameter();
    auto block{ std::make_shared<Paragraph>() };
    const auto it = std::next(blocks_.begin(), static_cast<std::ptrdiff_t>(index));
    blocks_.insert(it, block);
    return block;
  }

  ParagraphPointer Container::insertParagraph(const size_t index, ParagraphProperties prop)
  {
    auto block{ insertParagraph(index) };
    block->prop_ = std::move(prop);
    return block;
  }

  TablePointer Container::addTable(const size_t rows, const size_t cols)
  {
    auto block{ std::make_shared<Table>(rows, cols) };
    blocks_.push_back(block);
    return block;
  }

  TablePointer Container::insertTable(const size_t index, const size_t rows, const size_t cols)
  {
    if (index > blocks_.size())
      throw invalid_parameter();
    auto block{ std::make_shared<Table>(rows, cols) };
    const auto it = std::next(blocks_.begin(), static_cast<std::ptrdiff_t>(index));
    blocks_.insert(it, block);
    return block;
  }

  void Container::deleteBlock(const BlockPointer& block)
  {
    block->destroy();
    blocks_.remove(block);
  }

  bool Container::deleteBlockAt(const size_t index)
  {
    if (index >= blocks_.size())
      return false;
    auto it = std::next(blocks_.begin(), static_cast<std::ptrdiff_t>(index));
    (*it)->destroy();
    blocks_.erase(it);
    return true;
  }

  void Container::clear()
  {
    for (auto& block : blocks_)
      block->destroy();
    blocks_.clear();
  }
}

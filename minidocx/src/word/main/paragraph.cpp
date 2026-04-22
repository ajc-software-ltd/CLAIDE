// ============================================================================
// MINIDOCX
// ============================================================================
// File:        paragraph.cpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#include "word/main/paragraph.hpp"
#include "word/main/richtext.hpp"
#include "word/main/picture.hpp"
#include "utils/exceptions.hpp"


namespace MINIDOCX_NAMESPACE
{
  RunPointer Paragraph::runAt(const size_t index) const
  {
    if (index >= runs_.size())
      throw invalid_parameter();
    return *std::next(runs_.begin(), static_cast<std::ptrdiff_t>(index));
  }

  RichTextPointer Paragraph::addRichText(const char* text)
  {
    auto run{ std::make_shared<RichText>(text) };
    runs_.push_back(run);
    return run;
  }

  RichTextPointer Paragraph::addRichText(std::string text)
  {
    auto run{ std::make_shared<RichText>(std::move(text)) };
    runs_.push_back(run);
    return run;
  }

  RichTextPointer Paragraph::insertRichText(const size_t index, std::string text)
  {
    if (index > runs_.size())
      throw invalid_parameter();
    auto run{ std::make_shared<RichText>(std::move(text)) };
    const auto it = std::next(runs_.begin(), static_cast<std::ptrdiff_t>(index));
    runs_.insert(it, run);
    return run;
  }

  PicturePointer Paragraph::addPicture(const RelationshipId id)
  {
    auto run{ std::make_shared<Picture>(id) };
    runs_.push_back(run);
    return run;
  }

  PicturePointer Paragraph::insertPicture(const size_t index, const RelationshipId id)
  {
    if (index > runs_.size())
      throw invalid_parameter();
    auto run{ std::make_shared<Picture>(id) };
    const auto it = std::next(runs_.begin(), static_cast<std::ptrdiff_t>(index));
    runs_.insert(it, run);
    return run;
  }

  void Paragraph::deleteRun(const RunPointer& run)
  {
    run->destroy();
    runs_.remove(run);
  }

  bool Paragraph::deleteRunAt(const size_t index)
  {
    if (index >= runs_.size())
      return false;
    auto it = std::next(runs_.begin(), static_cast<std::ptrdiff_t>(index));
    (*it)->destroy();
    runs_.erase(it);
    return true;
  }

  void Paragraph::clear()
  {
    for (auto& run : runs_)
      run->destroy();
    runs_.clear();
  }
}

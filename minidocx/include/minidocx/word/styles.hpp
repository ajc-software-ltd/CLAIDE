// ============================================================================
// MINIDOCX
// ============================================================================
// File:        styles.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/properties/paragraph.hpp"
#include "word/main/properties/richtext.hpp"


namespace MINIDOCX_NAMESPACE
{
  struct StyleDefinition
  {
    std::string name_;
    std::string basedOn_;
  };

  struct CharacterStyle : StyleDefinition, RichTextProperties
  {
  };

  struct ParagraphStyle : CharacterStyle, ParagraphProperties
  {
    std::string next_;
  };

}

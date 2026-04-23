// ============================================================================
// MINIDOCX
// ============================================================================
// File:        paragraph.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/properties/base.hpp"


namespace MINIDOCX_NAMESPACE
{
  struct ParagraphProperties
  {
    std::string style_;


    // Paragraph Alignment 
    std::optional<Alignment> align_;


    // Outline Level 
    // Specifies the outline level associated with the paragraph.
    // It is used to build the table of contents and does not affect
    // the appearance of the text.
    // 
    enum class OutlineLevel {
      Level1, Level2, Level3, Level4, Level5, Level6, Level7, Level8, Level9, BodyText,
    } outlineLevel_ = OutlineLevel::BodyText;


    enum class SpecialIndentationType {
      None,

      // Specifies the additional indentation which shall be applied
      // to the first line of the parent paragraph.
      // This additional indentation is specified relative to the paragraph indentation
      // which is specified for all other lines in the parent paragraph.
      // 
      FirstLine,

      // Specifies the indentation which shall be removed from the first line
      // of the parent paragraph, by moving the indentation on the first line
      // back towards the beginning of the direction of text flow.
      // This indentation is specified relative to the paragraph indentation
      // which is specified for all other lines in the parent paragraph.
      // 
      Hanging
    };


    // Paragraph Indentation 
    struct Indentation {
      // Left/Right
      // /
      struct {
        // Specifies the indentation in hundreths of a character (100 = 1 character)
        // or absolute units (1440 = 1 Inch = 72 points).
        // 100 = 1 
        // 1440 = 1  = 72 
        bool chars_ = false;
        size_t value_ = 0;
      } left_, right_;

      // Special
      // 
      struct {
        SpecialIndentationType type_ = SpecialIndentationType::None;
        bool chars_ = true;
        size_t value_ = 0;
      } special_;
    };

    std::optional<Indentation> indent_;


    enum class SpacingType {
      // Specifies that the spacing should be determined automatically
      // by the consumer/wordprocessor and the value is ignored.
      // 
      Auto,

      // Specifies the spacing in hundreths of a line (100 = 1 line).
      // 100 = 1 
      Lines,

      // Specifies the spacing in absolute units (1440 = 1 Inch = 72 points).
      // 1440 = 1  = 72 
      Absolute
    };


    enum class LineSpacingType {
      // Single / 1.5 lines / Double / Multiple (240 = 1 line)
      //  / 1.5  / 2  / 240 = 1 
      Lines,

      // At Least (240 = 12 pt)
      // 240 = 12 
      AtLeast,

      // Exactly (240 = 12 pt)
      // 240 = 12 
      Exactly
    };


    // Spacing 
    struct Spacing {
      // Spacing between paragraphs
      // /
      struct {
        SpacingType type_ = SpacingType::Auto;
        size_t value_ = 100;
      } before_, after_;

      // Spacing between lines of a paragaph
      // 
      struct {
        LineSpacingType type_ = LineSpacingType::Lines;
        size_t value_ = 240;
      } lineSpacing_;
    };

    std::optional<Spacing> spacing_;


    // Borders 
    using ParagraphBorders = OutsideBorders;
    std::optional<ParagraphBorders> borders_;


    // Keep with next 
    // Specifies that the paragraph (or at least part of it)
    // should be rendered on the same page as the next paragraph
    // when possible.
    bool keepNext_ = false;

    // Keep lines together 
    // Specifies that all lines of the paragraph are to be kept
    // on a single page when possible.
    bool keepLines_ = false;

    // Page break before 
    // Specifies that the contents of this paragraph are rendered
    // on the start of a new page.
    bool pageBreakBefore_ = false;

    // Page break after 
    // Specifies that the contents of the next paragraph are rendered
    // on the start of a new page.
    bool pageBreakAfter_ = false;
  };

}
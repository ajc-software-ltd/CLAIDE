// ============================================================================
// MINIDOCX
// ============================================================================
// File:        richtext.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/properties/base.hpp"


namespace MINIDOCX_NAMESPACE
{
  struct RichTextProperties
  {
    std::string style_;


    enum class FontTypeHint {
      Default, // No font hint
      EastAsia,
      ComplexScript
    };


    // Font 
    struct Font {

      // Font 
      std::string ascii_ = "Courier New";
      // ASCII (i.e., the first 128 Unicode code points)

      // Asian text font 
      std::string eastAsia_ = "Simsun";
      // East Asian (i.e., CJK)
      // 

      // High ANSI
      std::string hAnsi_ = "Times New Roman";

      // Complex Script (e.g., Arabic)
      // 
      std::string cs_ = "Times New Roman";

      // Font Type Hint 
      // Specifies the font type which shall be used to format any ambiguous characters
      // that can be mapped into multiple categories of the four mentioned above such as ellipsis.
      // 
      FontTypeHint hint_ = FontTypeHint::Default;
    };
    
    std::optional<Font> font_;


    // Font style 
    struct FontStyle {
      bool bold_ = false;   // 
      bool italic_ = false; // 
    } fontStyle_;


    // Font size 
    // Specifies the font size in half points (2 = 1 pt).
    // 2 = 1 
    size_t fontSize_ = 0;


    // Font color 
    // This color can either be presented as a hex value (in RRGGBB format), 
    // or auto to automatically choose an appropriate color based on the background.
    // RRGGBB  auto 
    std::string color_;


    enum class UnderlineStyle {
      None,
      Words,           // 

      Single,          // 
      Double,          // 
      Thick,           // 

      Dotted,          // 
      DottedHeavy,     // 

      Dash,            // 
      DashedHeavy,     // 

      DashLong,        // 
      DashLongHeavy,   // 

      DotDash,         // 
      DashDotHeavy,    // 

      DotDotDash,      // 
      DashDotDotHeavy, // 

      Wave,            // 
      WavyDouble,      // 
      WavyHeavy,       // 
    };


    // Underline 
    struct Underline {
      UnderlineStyle style_ = UnderlineStyle::None;
      std::string color_ = "auto";
    } underline_;


    enum class StrikeStyle {
      None,
      Single, // 
      Double, // 
    };

    enum class VertAlign {
      None,
      Superscript, // 
      Subscript,   // 
    };


    // Effects 
    struct Effects {

      // Strikethrough 
      StrikeStyle strike_ = StrikeStyle::None;

      // Subscript/Superscript /
      VertAlign vertAlign_ = VertAlign::None;
    } effects_;


    // Highlighting 
    enum class Highlight {
      None,
      Black,       // 
      White,       // 
      Red,         // 
      Green,       // 
      Blue,        // 
      Yellow,      // 
      Cyan,        // 
      Magenta,     // 
      DarkRed,     // 
      DarkGreen,   // 
      DarkBlue,    // 
      DarkYellow,  // 
      DarkCyan,    // 
      DarkMagenta, // 
      DarkGray,    // 
      LightGray,   // 
    } highlight_ = Highlight::None;


    // Scale 
    // Specifies the amount to stretch or compress each character.
    // Note the minimum value is 1% and the maximum is 600%.
    //  1-600
    size_t scale_ = 100;


    enum class SpacingType {
      Normal,    // 
      Expanded,  // 
      Condensed, // 
    };


    // Spacing 
    // Specifies the amount of character pitch to be added or removed after each character.
    // 
    struct Spacing {

      SpacingType type_ = SpacingType::Normal;

      // 20 = 1 pt, 72 pt = 1 inch
      // 20 = 1 72  = 1 
      size_t by_ = 20;

    } spacing_;


    enum class PositionType {
      Normal,  // 
      Raised,  // 
      Lowered, // 
    };


    // Position 
    // Specifies the amount by which text shall be raised or lowered.
    // 
    struct Position {

      PositionType type_ = PositionType::Normal;

      // 2 = 1 pt, 72 pt = 1 inch
      // 2 = 1 72  = 1 
      size_t by_ = 2;

    } position_;


    // Text Border 
    std::optional<BorderProperties> border_;


    // True if whitespace needs to be preserved. 
    bool whitespace_ = false;
  };

}
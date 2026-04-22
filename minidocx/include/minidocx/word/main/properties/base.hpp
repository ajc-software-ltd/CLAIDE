// ============================================================================
// MINIDOCX
// ============================================================================
// File:        base.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include <optional>
#include <string>


namespace MINIDOCX_NAMESPACE
{
  enum class Alignment {
    Left,       // 
    Centered,   // 
    Right,      // 
    Justified,  // 
    Distributed // 
  };

  enum class BorderStyle {
    Single,    // 
    Double,    // 
    Triple,    // 
    Dotted,    // 
    Dashed,    // 
    DotDash,   // 
    Wave,      // 
    DoubleWave // 
  };

  struct BorderProperties
  {
    BorderStyle style_ = BorderStyle::Single;
    size_t width_ = 4; // 8 = 1 pt
    std::string color_ = "auto"; // "auto" or "RRGGBB"
  };

  struct OutsideBorders
  {
    BorderProperties top_, bottom_, left_, right_;
  };

  struct InsideBorders
  {
    BorderProperties insideHorizontal_, insideVertical_;
  };

  struct TableBorders : OutsideBorders, InsideBorders {};
}

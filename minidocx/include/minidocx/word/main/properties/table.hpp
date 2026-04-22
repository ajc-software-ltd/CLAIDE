// ============================================================================
// MINIDOCX
// ============================================================================
// File:        table.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/properties/base.hpp"


namespace MINIDOCX_NAMESPACE
{
  struct TableProperties
  {
    // Table Layout 
    enum class Layout {
      // Uses the preferred widths on the table items to generate
      // the sizing of the table, but then uses the contents of
      // each cell to determine the final column widths.
      // 
      Auto,
      // Uses the preferred widths on the table items to generate
      // the final sizing of the table. The width of the table is
      // not changed regardless of the contents of the cells. 
      // 
      Fixed
    } layout_ = Layout::Auto;


    // Specifies the units of the width_ property. 
    enum class WidthType {
      Auto, // Specifies that width is determined by the overall table layout algorithm.
      Percent, // 50 = 1%
      Absolute // 1440 = 1 Inch = 72 points
    };

    // Table Width 
    struct Width {
      WidthType type_ = WidthType::Auto;
      size_t value_ = 5000;
    } width_;


    // Table Alignment 
    // This property will not affect the alignment of the table
    // if the table spans the entire width of the page. Also it
    // does not affect the justification of text within the cells
    // of the table.
    // 
    enum class Alignment {
      Left,   // Align To Leading Edge 
      Right,  // Align to Trailing Edge 
      Center, // Align Center 
    } align_ = Alignment::Left;

    // Table Borders 
    TableBorders borders_;
  };

}
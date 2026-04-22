// ============================================================================
// MINIDOCX
// ============================================================================
// File:        section.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/properties/base.hpp"


namespace MINIDOCX_NAMESPACE
{
  // Page sizes in tw
  // 
  //   mm    cm    in    pt    tw    emu
  //    1                          36000
  //          1                   360000
  // 25.4  2.54     1    72  1440 914400
  //                      1    20
  //                            1    635

  // A3
  const unsigned int A3_W = 16838;
  const unsigned int A3_H = 23811;

  // A4
  const unsigned int A4_W = 11906;
  const unsigned int A4_H = 16838;

  // Letter
  const unsigned int LETTER_W = 12240;
  const unsigned int LETTER_H = 15840;

  // Legal
  const unsigned int LEGAL_W = 12240;
  const unsigned int LEGAL_H = 20160;

  // Tabloid
  const unsigned int TABLOID_W = 15840;
  const unsigned int TABLOID_H = 24480;

  // Executive
  const unsigned int EXECUTIVE_W = 10440;
  const unsigned int EXECUTIVE_H = 15120;


  struct SectionProperties
  {
    // Section Type 
    enum class Type {
      NextPage, // 
    } type_ = Type::NextPage;


    // Page Size 
    // Specifies the page size in twentieths of a point (tw).
    // 
    struct Size {
      size_t width_ = A4_W;
      size_t height_ = A4_H;
    } size_;
    
    
    // Page Orientation 
    bool landscape_ = false;


    // Page Margins 
    // Specifies the page margins in twentieths of a point (tw).
    // 
    struct Margins {
      size_t top_ = 1440;
      size_t bottom_ = 1440;
      // 1440 = 1 in = 2.54 cm

      size_t left_ = 1800;
      size_t right_ = 1800;
      // 1800 = 1.25 in = 3.17 cm

      size_t header_ = 992;
      size_t footer_ = 851;
      // 992 = 1.75 cm
      // 851 = 1.5 cm

      // Page Gutter Spacing 
      size_t gutter_ = 0;
    } margins_;


    // Document grid 
    // This element specifies the settings for the document grid, 
    // which enables precise layout of full-width East Asian
    // language characters within a document by specifying the 
    // desired number of characters per line and lines per page
    // for all East Asian text content in this section.
    // 
    struct DocGrid {

      // Document Grid Type 
      enum class Type {
        Default,       // No Document Grid 
        Lines,         // Line Grid Only 
        LinesAndChars, // Line and Character Grid 
        SnapToChars,   // Character Grid Only 
      } type_ = Type::Lines;

      // Specifies the number of lines to be allowed on the document
      // grid for the current page assuming all lines have equal line
      // pitch applied to them. 
      size_t linePitch_ = 312;
      // 312 = 15.6 pt (about 44 lines)
      // 312 = 15.6  44 
    };

    std::optional<DocGrid> docGrid_;
  };
}
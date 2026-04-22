// ============================================================================
// MINIDOCX
// ============================================================================
// File:        section.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/container.hpp"
#include "word/main/properties/section.hpp"


namespace MINIDOCX_NAMESPACE
{
  class MINIDOCX_API Section : public Container
  {
  public:
    SectionProperties prop_;
  };
}

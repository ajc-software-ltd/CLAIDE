// ============================================================================
// MINIDOCX
// ============================================================================
// File:        picture.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "word/main/base.hpp"
#include "word/main/properties/picture.hpp"
#include "packaging/relationship.hpp"


namespace MINIDOCX_NAMESPACE
{
  class MINIDOCX_API Picture : public Run
  {
  public:
    Picture(const RelationshipId id) : Run(RunType::Picture), id_{ id } {}
    ~Picture() override = default;

    PictureProperties prop_;
    RelationshipId id_;
  };
}

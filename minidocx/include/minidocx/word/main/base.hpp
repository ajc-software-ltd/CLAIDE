// ============================================================================
// MINIDOCX
// ============================================================================
// File:        base.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================


#pragma once

#include "utils/base.hpp"


namespace MINIDOCX_NAMESPACE
{
  enum class BlockType { Paragraph, Table };
  enum class RunType { RichText, Picture };
  
  using Block = Node<BlockType>;
  using Run = Node<RunType>;

}

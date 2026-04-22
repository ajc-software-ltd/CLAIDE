#pragma once

// Public low-level model surface.
//
// Intended use:
// - direct, manual authoring in trusted code paths
// - foundational data model access used by all higher layers
//
// For deterministic workflow-style editing and analysis, prefer combining
// editing + inspection APIs from their dedicated headers.

#include "config.hpp"
#include "utils/exceptions.hpp"
#include "word/main/document.hpp"
#include "word/main/section.hpp"
#include "word/main/paragraph.hpp"
#include "word/main/richtext.hpp"
#include "word/main/picture.hpp"
#include "word/main/table.hpp"
#include "word/main/cell.hpp"

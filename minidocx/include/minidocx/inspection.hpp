#pragma once

// Public inspection/analysis surface.
//
// Intended use:
// - semantic querying and extraction
// - computed style resolution
// - renderer-neutral layout generation
//
// This layer is read-model oriented and is preferred for deterministic
// analysis workflows built on document state.

#include "inspection/semantic.hpp"
#include "inspection/style_resolution.hpp"
#include "inspection/layout.hpp"

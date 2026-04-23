// ============================================================================
// MINIDOCX
// ============================================================================
// File:        minidocx.hpp
// Project:     CLAIDE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#pragma once

// Umbrella public header.
//
// Contract guidance:
// 1) model.hpp is the foundational low-level mutable model surface.
// 2) editing.hpp + inspection.hpp are the preferred integration surface for
//    deterministic higher-level workflows.
//
// This header intentionally exports all public layers for convenience.

#include "config.hpp"
#include "model.hpp"
#include "editing.hpp"
#include "inspection.hpp"

#include "providers.hpp"

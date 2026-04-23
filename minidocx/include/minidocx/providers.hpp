#pragma once

// Optional companion provider surface.
//
// Python-backed providers are out-of-process and optional.
// Companion-only policy: core minidocx model/editing/inspection functionality remains authoritative.
// Provider results must retain provenance and never imply automatic core feature parity.

#include "providers/python_bridge.hpp"

# CLAIDE Lint PR Chunk Plan

This document tracks the 7-step lint workflow used to land lint improvements in reviewable chunks.

## Chunk sequence

1. Tooling baseline and lint report structure
2. Core/platform lint fixes
3. UI lint fixes
4. Vulkan lint fixes
5. Test-suite lint fixes
6. Documentation and workflow cleanup
7. Final validation + release summary

## Standard commands

```bash
# changed files only
cmake --build build --target lint-fast

# full lint
cmake --build build --target lint

# scoped full-scope lint chunks
cmake --build build --target lint-core
cmake --build build --target lint-ui
cmake --build build --target lint-vulkan
cmake --build build --target lint-tests
```

## Reporting

- Raw output: `build/reports/clang-tidy.changed.raw.txt`
- Unique output: `build/reports/clang-tidy.changed.unique.txt`
- Summary: `build/reports/clang-tidy.changed.summary.md`

Scoped full-scope chunk runs write:
- `build/reports/clang-tidy.core.*`
- `build/reports/clang-tidy.ui.*`
- `build/reports/clang-tidy.vulkan.*`
- `build/reports/clang-tidy.tests.*`

## Versioning per chunk

Starting from `0.0.125-dev`, increment the patch version for every lint chunk commit.

- Chunk A: `0.0.126-dev`
- Chunk B: `0.0.127-dev`
- Chunk C: `0.0.128-dev`
- Chunk D: `0.0.129-dev`
- Chunk E: `0.0.130-dev`
- Chunk F: `0.0.131-dev`
- Chunk G: `0.0.132-dev`

If additional follow-up work is needed after the planned chunks, that follow-up must use the next version (for example `0.0.133-dev`).


## Execution status

- ✅ Chunk A (`0.0.126-dev`) tooling baseline landed.
- ✅ Chunk B (`0.0.127-dev`) core/platform baseline landed.
- ✅ Chunk C (`0.0.128-dev`) UI lint scope executed.
- ✅ Chunk D (`0.0.129-dev`) Vulkan/render lint scope executed.
- ✅ Chunk E (`0.0.130-dev`) tests lint scope executed.
- ✅ Chunk F (`0.0.131-dev`) docs/workflow cleanup completed.
- ✅ Chunk G (`0.0.132-dev`) pre-final reconciliation completed.
- ✅ Final full-sweep release (`0.0.133-dev`) completed with zero lint errors (Bucket A = 0 across scopes).

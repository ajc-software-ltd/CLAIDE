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
```

## Reporting

- Raw output: `build/reports/clang-tidy.changed.raw.txt`
- Unique output: `build/reports/clang-tidy.changed.unique.txt`
- Summary: `build/reports/clang-tidy.changed.summary.md`

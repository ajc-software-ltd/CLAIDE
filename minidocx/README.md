minidocx is a modern, free, open-source, cross-platform, lightweight C++20 library for manipulating Microsoft Word documents (`.docx`) from code, without requiring MS Office/WPS Office.

This branch exposes a layered public surface: low-level model APIs plus higher-level command/query/style/layout APIs.

## Public API Contract (PR9)

- **Supported low-level workflow:** direct model mutation (`Document`/`Section`/`Paragraph`/`RichText`/etc.) for manual authoring in trusted code.
- **Preferred higher-level workflow:** command-based edits + inspection APIs for deterministic integration paths.
- **Recommended build-on-top surface for future adapter work:** `editing` + `inspection` first, model APIs as foundational support.

## Include Surface

- `#include "minidocx/minidocx.hpp"`
  - umbrella include (all public layers)
- `#include "minidocx/model.hpp"`
  - low-level model-focused include
- `#include "minidocx/editing.hpp"`
  - command editing include
- `#include "minidocx/inspection.hpp"`
  - semantic/style/layout analysis include

## Current Capability Summary

### Core document model + DOCX read/write subset
- Document / section / paragraph / rich-text / table / picture model
- Supported subset authoring and round-trip load/save for current branch scope

### I/O modes
- File I/O: `load`, `saveAs`
- Stream I/O: `loadFromStream`, `saveToStream`
- Buffer I/O: `loadFromBuffer`, `saveToBuffer`

### Semantic inspection/query APIs
- Document statistics and listings (`inspection::summarize`, `list*`)
- Text extraction (`inspection::extractVisibleText`)

### Computed-style resolution
- Paragraph/run computed formatting resolution
- Deterministic issue reporting for missing references/cycles

### Neutral layout generation
- `inspection::buildLayout` produces page/content/node geometry structures
- Intended as renderer-neutral layout data

### Command-based editing
- `editing::applyCommand` with command variants for structure/text/style/numbering/table/image updates

## Preferred Usage Guidance

- **Direct model mutation** is suitable for low-level/manual authoring in trusted code.
- **Command + inspection APIs** are preferred for deterministic higher-level workflows.

## Out of Scope in This Branch (Current)

- CLAIDE adapter integration
- Vulkan/canvas bridge implementation
- AI endpoint wiring
- Full Microsoft Word parity
- New DOCX families not yet supported here, including:
  - comments
  - tracked revisions
  - footnotes / endnotes
  - headers / footers
  - text boxes
  - charts
  - equations
  - mail merge

## Examples

Examples are in `minidocx/examples/` and include:
- document creation/styling/media/table/list samples
- inspection + command workflow sample (`inspection_workflow.cpp`)

## Building

To build minidocx you need C++20 and CMake 3.28.

```bash
cd minidocx

# Linux
cmake --preset x64-linux-ninja-gcc
cmake --build --preset x64-linux-ninja-gcc-debug

# Windows
cmake --preset x64-win-msbuild-v143
cmake --build --preset x64-win-msbuild-v143-debug
```

A static library is built by default. To build shared, set `BUILD_SHARED=ON`.

## Documentation

- [User Guide](./guide.md)
- [Branch Status](./BRANCH_STATUS.md)

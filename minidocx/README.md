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

## Optional Python Provider Bridge (PR11)

`minidocx` now supports an optional out-of-process Python provider bridge.

- Core C++ model/editing/inspection/layout remains authoritative.
- Python providers are companion capabilities and are disabled by default (`MINIDOCX_ENABLE_PYTHON_BRIDGE=OFF`).
- Current provider set:
  - `smoke.ping` (bridge health check)
  - `mammoth.docx_to_html` (semantic DOCX -> HTML export helper)
  - `docxcompose.compose_append` (append one DOCX into another)
  - optional expert XML workflows via `lxml.xpath_query` / `lxml.xslt_transform`
    - allowlisted parts only: `word/document.xml`, `word/styles.xml`, `word/numbering.xml`,
      `_rels/.rels`, `word/_rels/document.xml.rels`, `[Content_Types].xml`
    - explicit JSON request contract with operation + part + output mode
    - structured JSON result payload with provenance and normalized warnings/errors
  - optional OCR image text extraction via `ocr.extract_text` (pytesseract + Tesseract runtime)
    - image input path or base64 image payload
    - constrained option surface (`lang`, allowlisted `psm`)
    - explicit normalized provider errors for missing runtime/language data

Bridge failures are surfaced as explicit response codes; missing Python/provider dependencies do not break core engine use.

Bridge hardening guarantees (PR12):

- Explicit launch strategy only: either `workerExecutablePath`, or `pythonExecutablePath + workerScriptPath`
- Protocol version + schema validation on request/response
- Deterministic normalized bridge error surface
- Capability probe reports provider availability, versions, and capabilities before execution
- Result provenance indicates provider-assisted origin

## Python Providers: Template Rendering and Style Audit (PR13)

Added optional second-wave providers (still companion-only):

- `docxtpl.render_template`
  - single-template + single-context + single-output DOCX rendering
  - not mail merge / not bulk generation
- `python_docx.style_audit`
  - style inventory/reporting helper for diagnostics and compatibility checks

Both providers return explicit provenance through the PR12 bridge contract and do not replace native minidocx model/editing/inspection behavior.

## Python Provider: Expert XML/XPath/XSLT (PR15)

`lxml` support is intentionally expert/advanced and companion-only:

- `lxml.xpath_query`: run XPath against an allowlisted DOCX XML part
- `lxml.xslt_transform`: run controlled XSLT transform against an allowlisted DOCX XML part
- not a general scripting surface and not a replacement for native minidocx editing/inspection semantics

## Python Provider: OCR Image Text Extraction (PR16)

`ocr.extract_text` is optional, companion-only OCR support backed by `pytesseract` and Tesseract:

- image in (path or base64 payload) -> extracted text out
- explicit provider availability/runtime dependency reporting
- normalized bridge error contract for missing engine/language/runtime data

This does **not** imply native minidocx OCR support, full scanned-document workflows, PDF OCR pipelines, or
document-intelligence features.

## Validation & Readiness Gate (PR10)

`minidocx` now defines a branch-local readiness contract.

- Normal engine validation: build tests and run `minidocx_validate`
- Pre-integration readiness check: run `minidocx_preintegration_gate`

See [INTEGRATION_GATE.md](./INTEGRATION_GATE.md) for required checks and non-goals.

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
- [Integration Gate](./INTEGRATION_GATE.md)

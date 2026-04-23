minidocx is a modern, free, open-source, cross-platform, lightweight C++20 library for manipulating Microsoft Word documents (`.docx`) from code, without requiring MS Office/WPS Office.

This branch exposes a layered public surface: low-level model APIs plus higher-level command/query/style/layout APIs.

`minidocx` is intended as CLAIDE's text/document engine module and is built to be consumed as a static or shared library dependency by parent projects.

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
- Provider taxonomy (companion-only):
  - **DOCX companion providers**
    - `smoke.ping`
    - `mammoth.docx_to_html`
    - `docxcompose.compose_append`
    - `docxtpl.render_template`
    - `python_docx.style_audit`
  - **XML expert/validation providers**
    - `lxml.xpath_query`
    - `lxml.xslt_transform`
    - `schematron.validate_part`
    - allowlisted DOCX XML parts only (`word/document.xml`, `word/styles.xml`, `word/numbering.xml`,
      `_rels/.rels`, `word/_rels/document.xml.rels`, `[Content_Types].xml`)
  - **Image OCR providers**
    - `ocr.extract_text` (pytesseract + Tesseract runtime)
    - bounded options (`lang`, allowlisted `psm`)
  - **PDF companion providers**
    - `pypdf.extract_text` (minimal plain/layout extraction)
    - `pdfminer.extract_text` / `pdfminer.extract_layout` (advanced text/layout analysis)
    - scanned/image-only PDFs may require OCR in a separate workflow

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

## Python Provider: Schematron Validation (PR17)

`schematron.validate_part` is optional expert validation tooling backed by `lxml.isoschematron`:

- validate one allowlisted DOCX XML part per request
- schema supplied as text or file path (optional phase)
- structured validation result with `valid` plus normalized failed-assert/report data

This remains companion-only and does not mutate the minidocx native model automatically.

## Python Provider: Minimal PDF Text Extraction (PR18)

`pypdf.extract_text` is an optional companion provider for lightweight PDF text extraction:

- input: PDF path (and optional payload-based PDF bytes)
- modes: `plain` (default) and `layout`
- output: structured text extraction payload with provenance and normalized warnings/errors

If a PDF is scanned/image-only, extracted text may be empty/minimal and OCR may be required.
PR18 does not add OCR fallback, rendering, or advanced PDF analysis.

## Python Provider: Advanced PDF Text and Layout Analysis (PR19)

`pdfminer.extract_text` / `pdfminer.extract_layout` provide optional advanced analysis capabilities:

- high-level text extraction with optional page subset + bounded LAParams controls
- page-wise layout summaries from `extract_pages` (text box/line/char-level counts and snippets)
- structured payloads with explicit provenance and normalized warnings/errors

PR19 is analysis-only and does not add OCR, rendering, or PDF editing workflows.

## Validation & Readiness Gate (PR10)

`minidocx` now defines a branch-local readiness contract.

- Normal engine validation: build tests and run `minidocx_validate` (`minidocx.regression`, `minidocx.inspection_pipeline`, `minidocx.commands`, `minidocx.shared_consumer_smoke`, `minidocx.python_bridge`)
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
- provider umbrella smoke example (`python_providers.cpp`)
- provider family-focused examples:
  - `python_providers_docx_companions.cpp`
  - `python_providers_xml_expert.cpp`
  - `python_providers_media_pdf.cpp`

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

Shared-library artifact expectations:
- Windows: `minidocx.dll` (plus import library)
- Linux: `libminidocx.so`

Examples/tests are optional consumers (`BUILD_EXAMPLES`, `BUILD_TESTS`) and are not required for parent-project library consumption.


## Feature Freeze and Merge-Readiness (PR24)

PR25 sync note: branch has been updated with current `main` state for merge-conflict readiness while preserving the frozen minidocx contract.

`minidocx` is now in a feature-complete freeze phase for the current branch scope.

- No new engine/provider capability expansion is intended during this phase unless required by a merge blocker.
- Current contract is frozen for merge-prep: native engine authoritative, Python providers optional companion-only, shared-library consumption supported.
- Merge-readiness checklist (blockers vs non-blockers, required evidence): [MERGE_READINESS.md](./MERGE_READINESS.md).

## Documentation

- [User Guide](./guide.md)
- [Branch Status](./BRANCH_STATUS.md)
- [Integration Gate](./INTEGRATION_GATE.md)
- [Merge Readiness](./MERGE_READINESS.md)

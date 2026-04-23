# User Guide

This guide documents the currently supported minidocx branch capabilities.
`minidocx` is intended to be consumed as CLAIDE's authoritative text/document engine module.
It is intentionally branch-scoped and does not claim full Microsoft Word parity.


## Feature Freeze and Merge-Readiness (PR24)

PR25 sync note: merge-conflict readiness was verified after syncing current `main` into this branch without adding new minidocx capabilities.

The current branch scope is now in feature-freeze mode for merge preparation.

- Do not add new engine/provider capabilities in this phase unless required by a documented merge blocker.
- Preserve the current contract: native engine authority, editing+inspection preferred high-level surface, Python bridge companion-only and optional.
- Use [MERGE_READINESS.md](./MERGE_READINESS.md) as the merge checklist and blocker/non-blocker tracker.

## Recommended Usage Style

- **Direct mutation (Document/Section/Paragraph/Run APIs):** use for low-level, manual authoring where caller code fully controls the model.
- **Command + inspection APIs:** prefer for deterministic higher-level workflows, scripted edits, and post-edit verification.

In short: direct mutation is valid for trusted authoring paths; command/query is preferred for repeatable workflow logic.

## Public Layer Entry Points

Use the include surface that matches your intent:

- `#include "minidocx/minidocx.hpp"` for all public layers
- `#include "minidocx/model.hpp"` for low-level model APIs
- `#include "minidocx/editing.hpp"` for command-based mutation
- `#include "minidocx/inspection.hpp"` for semantic/style/layout analysis

For future integration-facing workflows, prefer `editing` + `inspection` as the primary contract surface and use model mutation where low-level control is intentionally required.

## Optional Python Provider Bridge (PR11)

The Python bridge is an optional out-of-process provider layer.

### Build mode

- Disabled by default: `-DMINIDOCX_ENABLE_PYTHON_BRIDGE=OFF`
- Enable bridge: `-DMINIDOCX_ENABLE_PYTHON_BRIDGE=ON`

### Provider taxonomy

- **DOCX companion providers**
  - `smoke.ping`
  - `mammoth.docx_to_html`
  - `docxcompose.compose_append`
  - `docxtpl.render_template`
  - `python_docx.style_audit`
- **XML expert/validation providers**
  - `lxml.xpath_query` / `lxml.xslt_transform`
  - `schematron.validate_part`
  - bounded to allowlisted DOCX XML parts
- **Image OCR providers**
  - `ocr.extract_text` (pytesseract/Tesseract)
  - constrained option surface (`lang`, allowlisted `psm`)
- **PDF companion providers**
  - `pypdf.extract_text` (minimal extraction)
  - `pdfminer.extract_text` / `pdfminer.extract_layout` (advanced analysis)
  - scanned/image-only PDFs may require OCR outside provider scope

Python dependencies are discovered at runtime by the worker and reported deterministically as availability or provider-unavailable errors.

### Explicit launch configuration

PR12 requires explicit worker launch configuration.

Use either:
- `workerExecutablePath`
- or `pythonExecutablePath` + `workerScriptPath`

No implicit current-working-directory or hidden fallback scanning is part of the public bridge contract.

### Capability probing and error normalization

### PR13 provider operations

- `docxtpl.render_template`
  - input: template docx path + JSON context payload
  - output: rendered docx path
  - bounded to single-template/single-context/single-output flow
- `python_docx.style_audit`
  - input: docx path
  - output: structured style inventory/report JSON

Both are optional provider-backed companion features and keep native minidocx engine responsibilities unchanged.

### PR15 expert XML provider operations

- `lxml.xpath_query`
  - input: DOCX path + JSON payload (`part`, `xpath`, optional `namespaces`, optional `mode`)
  - part must be one of:
    - `word/document.xml`
    - `word/styles.xml`
    - `word/numbering.xml`
    - `_rels/.rels`
    - `word/_rels/document.xml.rels`
    - `[Content_Types].xml`
  - output: structured JSON result payload (provider metadata, selected part, normalized matches)
- `lxml.xslt_transform`
  - input: DOCX path + JSON payload (`part`, `xslt`, optional `params`, `output_mode`)
  - output: structured JSON transform payload with provenance and normalized errors

### PR16 OCR provider operation

- `ocr.extract_text`
  - input: image path (`input_path`) or JSON payload image (`image_b64`)
  - optional options: `lang`, `psm` (bounded subset)
  - output: structured JSON payload containing extracted text + provider metadata/provenance
  - runtime dependency: Tesseract binary + language data must be installed

### PR17 Schematron validation provider operation

- `schematron.validate_part`
  - input: DOCX `input_path` + JSON payload (`part`, `schema_text` or `schema_path`, optional `phase`, optional `store_report`)
  - output: structured JSON validation payload containing `valid`, failed assertions, report entries, and provenance
  - constrained to allowlisted XML parts only
  - validation-only companion feature (no automatic mutation/fix-up behavior)

### PR18 minimal PDF extraction provider operation

- `pypdf.extract_text`
  - input: PDF `input_path` (or payload PDF bytes where available)
  - optional mode: `plain` or `layout`
  - output: structured JSON payload containing extracted text + provider metadata/provenance
  - limitation: scanned/image-only PDFs may require OCR; this provider does not perform OCR

### PR19 advanced PDF analysis provider operations

- `pdfminer.extract_text`
  - input: PDF `input_path` (or payload PDF bytes where available)
  - optional controls: `page_numbers`, bounded `laparams`
  - output: structured JSON payload containing extracted text + provider metadata/provenance
- `pdfminer.extract_layout`
  - input: PDF `input_path` (or payload PDF bytes where available)
  - output: structured page summaries (page bbox, text boxes, line/char counts, snippets)
  - non-goal: OCR/rendering/editing or broad PDF workflow expansion

Always call `probePythonProviders` before provider execution when bridge mode is enabled.
The bridge normalizes failures into a stable error taxonomy (`BridgeUnavailable`, `WorkerLaunchFailed`, `ProviderUnavailable`, `ProtocolMismatch`, `MalformedResponse`, etc.).

Bridge responses include provenance metadata so provider-assisted outputs are not confused with native core engine behavior.

### Provider examples

- umbrella smoke example: `examples/python_providers.cpp`
- family-focused examples:
  - `examples/python_providers_docx_companions.cpp`
  - `examples/python_providers_xml_expert.cpp`
  - `examples/python_providers_media_pdf.cpp`

## Branch Validation Flow (PR10)

Use the branch gate commands for repeatable health checks:

```bash
cmake -S minidocx -B minidocx/out/gate -G Ninja -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build minidocx/out/gate --target minidocx_validate
cmake --build minidocx/out/gate --target minidocx_preintegration_gate
```

- `minidocx_validate` runs the branch test suite (`regression`, `inspection_pipeline`, `commands`, `shared_consumer_smoke`, `python_bridge`).
- `minidocx_preintegration_gate` is the explicit pre-integration readiness alias.

For policy details and non-goals, see [INTEGRATION_GATE.md](./INTEGRATION_GATE.md).

## Measuring Units

minidocx uses point (`pt`), twentieth of a point (`tw`), and English Metric Unit (`emu`) for page, paragraph, and drawing geometry.

|   mm |   cm |   in |   pt |   tw |    emu |
| ---: | ---: | ---: | ---: | ---: | -----: |
|    1 |      |      |      |      |  36000 |
|      |    1 |      |      |      | 360000 |
| 25.4 | 2.54 |    1 |   72 | 1440 | 914400 |
|      |      |      |    1 |   20 |  12700 |
|      |      |      |      |    1 |    635 |

## Headers and Namespace

Use the umbrella header:

```cpp
#include "minidocx/minidocx.hpp"
using namespace md;
```

When linking through the `minidocx` CMake target, shared/static compile definitions are provided by target usage requirements; manual source-level `MINIDOCX_SHARED` defines are not required.

## Error Handling

minidocx throws `md::Exception` on failures in core load/save/model operations.
Command workflow APIs return `editing::CommandResult` for explicit success/error handling.

```cpp
try {
  // core API usage
}
catch (const md::Exception& ex) {
  std::cerr << ex.what() << '\n';
}
```

## Core Document Model

A document is represented by `Document`, with nested sections and block/run content:

- Document
  - Section
    - Paragraph
      - RichText
      - Picture
    - Table
      - Cell

### Create and save

```cpp
md::Document doc;
auto section = doc.addSection();
section->addParagraph()->addRichText("Hello");
doc.saveAs("example.docx");
```

### Load/save I/O modes

```cpp
md::Document doc;
doc.load("in.docx");

auto bytes = doc.saveToBuffer();
md::Document fromBuffer;
fromBuffer.loadFromBuffer(bytes);

std::stringstream ss;
doc.saveToStream(ss);
md::Document fromStream;
fromStream.loadFromStream(ss);
```

## Semantic Inspection / Query

Inspection APIs are in `md::inspection`:

- `summarize`
- `listSections`, `listParagraphs`, `listOutlineParagraphs`
- `listRuns`, `listTables`, `listCells`, `listPictures`
- `extractVisibleText`, `extractSectionVisibleText`
- `hasNumbering`

Example:

```cpp
const auto stats = md::inspection::summarize(doc);
const auto text = md::inspection::extractVisibleText(doc);
```

## Computed-Style Resolution

Style resolution APIs compute effective paragraph/run formatting based on style chains, numbering, and direct properties:

- `resolveParagraphFormatting(document, paragraph)` or by `NodePath`
- `resolveRunFormatting(document, paragraph, run)` or by `NodePath`

These APIs also return `issues` for missing references and style cycles.

## Neutral Layout Generation

`md::inspection::buildLayout(doc)` returns a renderer-neutral `DocumentLayout` containing:

- pages with page/content rectangles
- line-level geometry
- node references (paragraph/table/cell/picture/run)

This layer provides deterministic geometry data and does not implement renderer integration itself.

## Command-Based Editing

`md::editing::applyCommand(document, command)` applies a typed edit and returns `CommandResult`.

Representative command groups include:

- structure: insert/delete section/paragraph/block
- text/runs: replace paragraph text, insert/replace run text
- media/tables: insert picture, create table, merge/split cell
- styling: apply paragraph/character style, set paragraph/run properties
- numbering/section: apply numbering, update section properties

Example:

```cpp
md::editing::CommandResult r = md::editing::applyCommand(
    doc, md::editing::ReplaceParagraphTextCommand{{0, 0, 0, false}, "Updated"});
if (!r.success) {
  std::cerr << r.message << '\n';
}
```

## Current Non-Goals (Branch Scope)

The current minidocx branch does **not** include:

- CLAIDE adapter/UI/editor integration
- Vulkan/canvas bridge implementation
- AI endpoint wiring
- full Microsoft Word parity
- additional DOCX part/story families such as comments, tracked revisions,
  footnotes/endnotes, headers/footers, charts, equations, text boxes, mail merge

For branch progress and next chunk direction, see [BRANCH_STATUS.md](./BRANCH_STATUS.md).

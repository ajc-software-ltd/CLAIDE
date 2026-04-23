# User Guide

This guide documents the currently supported minidocx branch capabilities.
It is intentionally branch-scoped and does not claim full Microsoft Word parity.

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

### Current bridge providers

- `smoke.ping` -> returns `pong`
- `mammoth.docx_to_html` -> semantic HTML export helper
- `docxcompose.compose_append` -> append/compose DOCX output
- optional expert XML operations through `lxml.xpath_query` and `lxml.xslt_transform`
  - bounded to allowlisted DOCX package parts only
  - explicit JSON payload contract (`part`, operation-specific fields, optional params)
  - structured JSON output payload with provenance + normalized warnings/errors
- optional OCR image-text extraction through `ocr.extract_text`
  - requires pytesseract + installed Tesseract runtime
  - supports image path input or base64 image payload
  - option surface remains constrained (`lang`, allowlisted `psm`)

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

Always call `probePythonProviders` before provider execution when bridge mode is enabled.
The bridge normalizes failures into a stable error taxonomy (`BridgeUnavailable`, `WorkerLaunchFailed`, `ProviderUnavailable`, `ProtocolMismatch`, `MalformedResponse`, etc.).

Bridge responses include provenance metadata so provider-assisted outputs are not confused with native core engine behavior.

## Branch Validation Flow (PR10)

Use the branch gate commands for repeatable health checks:

```bash
cmake -S minidocx -B minidocx/out/gate -G Ninja -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build minidocx/out/gate --target minidocx_validate
cmake --build minidocx/out/gate --target minidocx_preintegration_gate
```

- `minidocx_validate` runs the branch test suite (`regression`, `inspection_pipeline`, `commands`).
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

If you link against a shared build, define `MINIDOCX_SHARED` before including the header.

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

# User Guide

This guide documents the currently supported minidocx branch capabilities.
It is intentionally branch-scoped and does not claim full Microsoft Word parity.

## Recommended Usage Style

- **Direct mutation (Document/Section/Paragraph/Run APIs):** use for low-level, manual authoring where caller code fully controls the model.
- **Command + inspection APIs:** prefer for deterministic higher-level workflows, scripted edits, and post-edit verification.

In short: direct mutation is valid for trusted authoring paths; command/query is preferred for repeatable workflow logic.

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

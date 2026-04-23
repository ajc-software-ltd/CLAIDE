# MiniExcelX PR2 Scope Lock

## Objective
PR2 adds worksheet content reading to MiniExcelX: worksheet XML parsing, shared string resolution, and a minimal typed cell model with deterministic row/cell ordering.

## Included in PR2
- Worksheet XML parsing for discovered worksheet parts.
- Shared strings table resolution when present.
- Minimal typed `Cell` model (`blank`, `string`, `inline string`, `number`, `boolean`, `error`, `formula cached shell`).
- Worksheet read API for cells and used-content counters.
- Engine tests covering shared strings, inline strings, numeric/boolean values, blank handling, and stable ordering.

## Explicit Non-Scope for PR2
- Save/write/serialization paths.
- Formula feature work (evaluation/preservation APIs).
- Style/format interpretation, merged cells, tables, charts, pivots.
- CLAIDE host integration (`MiniexcelAdapter`, `OfficeSpreadsheetService`, `SpreadsheetLayoutBridge`).
- UI/render/Vulkan/AI wiring and Python bindings.

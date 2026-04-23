# MiniExcelX PR1 Scope Lock

## Objective
PR1 adds the first real MiniExcelX engine capability: open a valid `.xlsx` package, discover workbook structure, and enumerate sheets in stable workbook order.

## Included in PR1
- ZIP/OOXML package open path for `.xlsx`.
- Discovery of `[Content_Types].xml`, package root relationships, workbook part, workbook relationships, and ordered `<sheet>` entries.
- Minimal metadata model updates for `Workbook` and `Worksheet` needed for discovery.
- Deterministic failure handling for malformed/unsupported inputs.
- Engine tests for valid open/discovery and negative-path rejection behavior.

## Explicitly Unsupported in PR1
- `.xls` legacy format.
- Encrypted/password-protected Excel packages.

## Explicit Non-Scope for PR1
- Worksheet row parsing, cell parsing, typed values, shared strings, styles, formulas.
- Save/write/serialization paths.
- CLAIDE host integration (`MiniexcelAdapter`, `OfficeSpreadsheetService`, `SpreadsheetLayoutBridge`).
- UI/render/Vulkan/AI wiring.
- Python bindings.

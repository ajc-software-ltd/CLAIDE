# MiniExcelX PR0 Scope Lock

## Purpose
MiniExcelX PR0 establishes a branch-local C++ module baseline on branch `miniexcelx`.

## Included in PR0
- `miniexcelx/` module root with `include/`, `src/`, `tests/`, and `docs/`.
- Minimal compileable MiniExcelX C++ library skeleton.
- Minimal compileable smoke test target proving the harness runs.
- Public stub-only API shell for `Workbook`, `Worksheet`, and `Cell`.

## Supported Direction
- Target file format direction is `.xlsx`.
- Engine-first implementation in pure C++.
- Optional Python bridge may be considered in later PRs.

## Explicitly Unsupported in PR0
- `.xls`.
- Encrypted/password-protected Excel files.
- `.xlsm`.

## Explicit Non-Scope for PR0
- OOXML parsing.
- ZIP package reading.
- Workbook load/read/write/serialize logic.
- Worksheet XML parsing, shared strings, styles, formulas, or formula engine.
- CLAIDE host integration code (`MiniexcelAdapter`, `OfficeSpreadsheetService`, `SpreadsheetLayoutBridge`).
- Changes under `src/core/office/` for spreadsheet integration.
- UI, render, Vulkan, AI orchestration, prompt/export wiring.
- Python bindings.
## Branch/Ref Naming Guardrail
- Use branch names directly: `main`, `minidocx`, `miniexcelx`.
- Do not require remote-tracking ref names (for example `origin/<branch>`) as part of PR0 scope or validation.


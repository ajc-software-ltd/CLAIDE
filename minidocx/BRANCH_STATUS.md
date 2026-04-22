# minidocx Branch Status

_Last updated: 2026-04-22_

## Branch Contract Snapshot

- PR0–PR7: complete on this branch.
- PR8 scope: documentation/examples/branch-contract alignment only.

## Supported Subset (Current Branch)

1. Core DOCX model and supported read/write subset
   - Document/section/paragraph/rich-text/table/picture/list/style primitives
   - Save/load with package-based DOCX handling for the currently implemented subset

2. I/O surfaces
   - file, stream, and buffer I/O paths

3. Semantic inspection/query
   - structure stats/listing APIs
   - visible-text extraction

4. Computed-style resolution
   - paragraph and run formatting resolution
   - deterministic issue reporting for missing references and style cycles

5. Neutral layout generation
   - page/content/node geometry structures via inspection layout APIs

6. Command-based editing
   - typed edit commands applied through a single command execution API

## Explicit Non-Goals (Current)

- CLAIDE adapter integration
- Office service/UI/editor integration work
- Vulkan/canvas bridge work
- AI endpoint/provider wiring
- full Microsoft Word parity
- additional DOCX families not yet in branch scope:
  - comments
  - tracked revisions
  - footnotes / endnotes
  - headers / footers
  - text boxes
  - charts
  - equations
  - mail merge

## Next Likely Chunks (After PR8)

- Continue branch-local documentation hardening for supported subset behavior and limitations
- Expand deterministic test coverage around existing inspection/style/layout/editing surfaces
- Keep integration concerns (CLAIDE adapter/render/UI/AI) explicitly out of minidocx branch-contract PRs until separately scoped

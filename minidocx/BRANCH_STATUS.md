# minidocx Branch Status

_Last updated: 2026-04-22_

## Branch Contract Snapshot

- PR0–PR7: complete on this branch.
- PR8: documentation/examples/branch-contract alignment complete.
- PR9: public API contract clarification and surface-boundary tightening (no new engine features).

## Public Usage Contract (Current)

- Low-level model mutation is supported and remains first-class for manual/trusted authoring flows.
- Command-based editing plus inspection APIs are the preferred surface for deterministic higher-level workflows.
- Style resolution and layout are analysis layers built on document state and intended for read-model/reporting/render-prep usage.
- Future adapter-style integration should build on `editing` + `inspection` first, using model APIs as foundational support.

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

## Next Likely Chunks (After PR9)

- Continue module-scoped tests and docs hardening around existing surfaces
- Keep contract language consistent across headers/docs/examples as APIs evolve
- Keep integration concerns (CLAIDE adapter/render/UI/AI) explicitly out of minidocx branch-contract PRs until separately scoped

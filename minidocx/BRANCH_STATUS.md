# minidocx Branch Status

_Last updated: 2026-04-23_

## Branch Contract Snapshot

- PR0–PR7: complete on this branch.
- PR8: documentation/examples/branch-contract alignment complete.
- PR9: public API contract clarification and surface-boundary tightening (no new engine features).
- PR10: pre-integration readiness gate and validation contract (no new engine features).
- PR11: optional out-of-process Python provider bridge (companion layer, not core replacement).
- PR12: Python bridge hardening (explicit pathing, probing, protocol validation, normalized errors, provenance).
- PR13: optional template-render and style-audit providers via hardened Python bridge.
- PR15: optional expert `lxml` provider surface for allowlisted DOCX-part XPath and XSLT workflows.
- PR16: optional OCR provider via pytesseract/Tesseract for image text extraction.
- PR17: optional Schematron validation provider for allowlisted DOCX XML parts.
- PR18: optional minimal PDF text-extraction provider via pypdf.

## Public Usage Contract (Current)

- Low-level model mutation is supported and remains first-class for manual/trusted authoring flows.
- Command-based editing plus inspection APIs are the preferred surface for deterministic higher-level workflows.
- Style resolution and layout are analysis layers built on document state and intended for read-model/reporting/render-prep usage.
- Future adapter-style integration should build on `editing` + `inspection` first, using model APIs as foundational support.


## Readiness Gate (Current)


## Optional Provider Bridge (Current)

- Bridge hardening: explicit launch config, capability probing, versioned protocol validation, normalized errors
- Bridge model: out-of-process Python worker with explicit request/response payloads
- Build default: disabled (`MINIDOCX_ENABLE_PYTHON_BRIDGE=OFF`)
- Companion providers: smoke, mammoth export, docxcompose append, optional allowlisted-part lxml expert workflows,
  optional OCR extract-text via Tesseract, optional Schematron validation, optional minimal pypdf PDF text extraction,
  docxtpl template rendering, python-docx style audit
- Core C++ engine remains authoritative when bridge is disabled or unavailable

- Normal branch health validation target: `minidocx_validate`
- Pre-integration readiness target: `minidocx_preintegration_gate`
- Canonical gate policy and command contract: [INTEGRATION_GATE.md](./INTEGRATION_GATE.md)

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

## Next Likely Chunks (After PR18)

- Continue module-scoped tests and docs hardening around existing surfaces
- Keep contract language consistent across headers/docs/examples as APIs evolve
- Keep integration concerns (CLAIDE adapter/render/UI/AI) explicitly out of minidocx branch-contract PRs until separately scoped

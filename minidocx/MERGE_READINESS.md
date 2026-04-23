# minidocx Merge Readiness Checklist (PR24)

_Last updated: 2026-04-23_

## Phase Declaration

`minidocx` is in a **feature-complete freeze** phase for the current branch scope.

- No new engine/provider features during this phase unless required by a merge blocker.
- This document defines what must be true before merge into `main` is attempted.

## Frozen Branch Contract

- Native C++ minidocx engine remains authoritative.
- `editing` + `inspection` remain preferred integration-facing surfaces.
- Python provider bridge remains optional and companion-only (`MINIDOCX_ENABLE_PYTHON_BRIDGE=OFF` by default).
- Shared-library consumption remains supported and validated (`minidocx.shared_consumer_smoke`).
- CLAIDE-side adapter/render/UI/AI integration remains out of scope in this phase.

## Merge Surface Inventory

1. **Library/build surface**
   - static/shared build paths via `BUILD_SHARED`
   - shared artifact naming expectations documented
2. **Public header surface**
   - umbrella/model/editing/inspection includes are contract surfaces
3. **Provider bridge surface (optional)**
   - companion-only provider taxonomy and bounded operations
4. **Validation/test surface**
   - `minidocx.regression`
   - `minidocx.inspection_pipeline`
   - `minidocx.commands`
   - `minidocx.shared_consumer_smoke`
   - `minidocx.python_bridge`
   - aggregate targets: `minidocx_validate`, `minidocx_preintegration_gate`
5. **Examples/docs surface**
   - examples and docs aligned to current contract and non-goals

## Required Evidence Before Merge

- Branch validation targets pass according to `INTEGRATION_GATE.md`.
- Validation surface in docs matches actual CMake target graph.
- No new capability claims beyond implemented/frozen scope.
- Non-goals remain respected (no CLAIDE-side integration in this branch phase).

## Merge Blockers vs Non-Blockers

### Merge blockers (must fix before merge)

- Validation target failures in `minidocx_validate` or `minidocx_preintegration_gate`.
- Contract mismatch between docs and actual build/test surfaces.
- Regressions in core model/editing/inspection/layout behavior for current scope.
- Changes that violate frozen non-goals by introducing CLAIDE integration work.

### Non-blockers (follow-up after merge)

- Additional provider expansion requests outside current frozen scope.
- Feature requests for unsupported DOCX families (comments, revisions, etc.).
- Future runtime plugin ABI/c-wrapper redesign discussions.
- CLAIDE adapter/render/UI/AI integration execution work (planned post-merge phases).

## Out of Scope During PR24

- New engine/provider features
- Broad refactors for style/polish only
- CLAIDE-side integration code
- Actual main sync/rebase/merge execution

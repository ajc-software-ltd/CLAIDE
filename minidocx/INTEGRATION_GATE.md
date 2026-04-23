# minidocx Pre-Integration Readiness Gate (PR10/PR11)

_Last updated: 2026-04-22_

## Purpose

This document defines the branch-local validation contract for `minidocx`.
It establishes what “healthy branch state” means and what must pass before any future CLAIDE integration phase is allowed to begin.

This is a minidocx-stream gate, not a CLAIDE-wide CI policy.

## Current Branch Guarantees

The current branch guarantees, for the implemented subset:

- core DOCX read/write behavior through existing model APIs
- semantic inspection/query APIs
- computed style resolution APIs
- neutral layout generation APIs
- command-based editing APIs
- deterministic branch regression/inspection/commands test coverage


## Feature Freeze and Merge-Readiness Phase (PR24)

For the current branch scope, `minidocx` is now in **feature-freeze** mode while merge readiness is audited.

- No new engine/provider capabilities are to be introduced in this phase unless required by a documented merge blocker.
- Scope contract remains frozen: native engine authoritative, editing+inspection preferred high-level surface, Python provider bridge optional companion-only, shared-library consumption supported.
- CLAIDE-side adapter/render/UI/AI integration remains out of scope in this phase.
- Merge-readiness checklist and blocker tracking are maintained in [MERGE_READINESS.md](./MERGE_READINESS.md).

## Validation Contract

### A) Required for normal branch health

All of the following must pass:

1. Build tests with `BUILD_TESTS=ON`
2. Execute branch validation suite:
   - `minidocx.regression`
   - `minidocx.inspection_pipeline`
   - `minidocx.commands`
   - `minidocx.shared_consumer_smoke`
   - `minidocx.python_bridge`
3. Public API contract remains consistent across:
   - public headers (`minidocx.hpp`, `model.hpp`, `editing.hpp`, `inspection.hpp`)
   - branch docs (`README.md`, `guide.md`, `BRANCH_STATUS.md`, this file)
4. No unsupported DOCX family support is newly claimed.

### B) Required before future CLAIDE integration work can begin

In addition to normal branch health checks:

1. `minidocx_preintegration_gate` target passes.
2. Branch non-goals remain respected (no adapter/render/UI/AI wiring).
3. No failing/disabled minidocx gate tests are carried forward.


### C) Python bridge checks (optional mode)

When building with `MINIDOCX_ENABLE_PYTHON_BRIDGE=ON`:

1. `minidocx.python_bridge` test must pass.
2. Probe and smoke operations must return deterministic results.
3. Missing third-party Python dependencies must return actionable provider-unavailable errors (not crashes).
4. Protocol version/schema validation failures must return deterministic bridge errors.
5. Provenance for provider-backed responses must be explicit.
6. PR13/PR15/PR16/PR17/PR18/PR19 provider operations remain bounded:
   - PR13: single-template render + style-audit only (no mail-merge expansion)
   - PR15: allowlisted DOCX XML-part XPath/XSLT only (no unrestricted archive traversal/scripting)
   - PR16: image OCR extract-text only via provider contract (no PDF/document-intelligence pipeline claims)
   - PR17: allowlisted DOCX XML-part Schematron validation only (no automatic mutation/fix-up workflows)
   - PR18: minimal PDF text extraction only via pypdf (no OCR fallback/rendering/advanced PDF analysis)
   - PR19: advanced PDF text/layout analysis only via pdfminer.six (no OCR/rendering/editing workflows)

Provider taxonomy used by branch docs/gate:
- DOCX companion providers
- XML expert/validation providers
- Image OCR providers
- PDF companion providers


### D) Shared-library consumption readiness checks (PR22)

When building with `BUILD_SHARED=ON`:

1. `minidocx` shared library is produced with expected platform naming.
2. Visibility/export behavior is explicit (`MINIDOCX_API` for shared-library import/export semantics across platforms).
3. `minidocx.shared_consumer_smoke` passes, proving public-header + target-link consumption.
4. `BUILD_EXAMPLES` and `BUILD_TESTS` remain optional and do not gate parent-project library usage.
5. `MINIDOCX_ENABLE_PYTHON_BRIDGE` remains optional and off by default.


### E) Main-branch sync readiness checks (PR25)

After syncing `main` into `minidocx`:

1. Validation targets (`minidocx_validate`, `minidocx_preintegration_gate`) still pass.
2. Frozen contract remains unchanged (no new engine/provider features introduced by sync work).
3. Readiness docs (`BRANCH_STATUS.md`, `INTEGRATION_GATE.md`, `MERGE_READINESS.md`, `README.md`, `guide.md`) remain internally consistent.

## Branch Validation Commands

```bash
# Configure with tests (core mode)
cmake -S minidocx -B minidocx/out/gate -G Ninja -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON

# Configure with optional Python bridge
cmake -S minidocx -B minidocx/out/gate-py -G Ninja -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON -DMINIDOCX_ENABLE_PYTHON_BRIDGE=ON

# Normal validation suite
cmake --build minidocx/out/gate --target minidocx_validate

# Pre-integration readiness gate
cmake --build minidocx/out/gate --target minidocx_preintegration_gate
```

## Explicit Pre-Integration Non-Goals

Passing this gate does **not** mean the branch has started CLAIDE integration.
The following remain out of scope:

- CLAIDE adapter (`src/core/office/*`) and Office service integration
- Vulkan/canvas bridge work
- UI/editor integration
- AI endpoint/provider wiring
- full Microsoft Word parity
- new DOCX families (comments, tracked revisions, footnotes, endnotes, headers,
  footers, text boxes, charts, equations, mail merge)

## Change Policy

PR10/PR11/PR12/PR13/PR15/PR16/PR17/PR18/PR19/PR20/PR21/PR22/PR23/PR24/PR25 are validation/process, optional-provider hardening, shared-library consumption-readiness, merge-readiness-audit, and main-sync-readiness only.
Do not use this gate document to introduce unsupported engine capability claims.

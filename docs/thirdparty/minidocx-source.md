# minidocx Source Record

- Upstream: `https://github.com/ajc-software-ltd/minidocx`
- Imported into CLAIDE root: `minidocx/`
- Imported branch: `next`
- Pinned commit: `09cde3f`
- Imported on: 2026-04-22
- License: MIT

## Usage policy in CLAIDE

`minidocx` is used as the baseline DOCX engine for Milestone 3.1.
CLAIDE extends functionality in project-owned integration layers (core services, AI contracts, and render-canvas adapters) without modifying upstream files unless required.

## Local extension boundary

- Keep upstream code in `minidocx/` as intact as possible.
- Place CLAIDE-specific adaptation in `src/core/office/` and related CLAIDE docs.
- Record any local patching of upstream code in this file.

# Milestone 3 (M3.x) Overview

Milestone 3 is split into focused sub-milestones to keep implementation controllable and testable.

## M3.x structure

- **M3.1** DOCX-first integration using `minidocx`
- **M3.2** XLSX implementation (deferred)
- **M3.3** SVG read/import/display pipeline
- **M3.4** TTF font pipeline (system/import/Google Fonts)
- **M3.5** PDF/AI interoperability (lower priority)
- **M3.6** Render-canvas unification for all document outputs
- **M3.7** AI operation contracts for document workflows
- **M3.8** Cross-platform hardening (CachyOS primary, Windows secondary)
- **M3.9** Release readiness and validation gate

## Global rule

All text/document outputs must be viewable through CLAIDE's Vulkan render surface.

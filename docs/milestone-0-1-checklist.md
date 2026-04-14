# Milestone 0/1 Completion Checklist

AJC-Software Ltd © 2026

This checklist is the practical signoff artifact for:
- **Milestone 1: Core IDE Shell** (complete)
- **Milestone 2 Vulkan Canvas foundation** (in progress)

> Source-of-truth status remains `milestones.md`.

---

## Milestone 1 — Core IDE Shell (Completed)

### Core functionality
- [x] Main application bootstrap and lifecycle
- [x] Dockable main UI shell with editor/media panels
- [x] Document + file service flow
- [x] UTF-8/UTF-16 parsing and validation improvements
- [x] Logging and crash-handling safeguards

### Validation
- [x] Catch2/CTest suite active
- [x] Build scripts available for Ubuntu preflight + build/test
- [x] Reproducible local build path documented in `README.md`

---

## Vulkan Runtime Foundation — Accounted Capabilities

### Runtime boundary
- [x] Public C ABI (`include/vulkanai/VulkanAI.h`)
- [x] Shared runtime target (`VulkanAI`)
- [x] Loader (`Core::VulkanRuntimeLoader`) with API compatibility checks

### Host integration
- [x] `RenderHost` abstraction
- [x] `VulkanRenderHost` adapter
- [x] UI runtime diagnostics/retry/error fallback in `MainFrame`

### Diagnostics completeness
- [x] Search paths shown
- [x] Failed load attempts shown
- [x] Reason code and human-readable name shown

---

## Remaining items before formal Milestone 2 closure

- [ ] Expand automated tests to fully cover runtime-host lifecycle edge cases
- [ ] CI-level enforcement of preflight-first build path
- [ ] Finalize milestone status narrative and publish milestone signoff

---

## Acceptance gates

### Milestone 1 (must remain green)
- [x] Editor save/save-as/delete flow available from UI
- [x] Dirty-state close confirmation active
- [x] Core encode/decode + safe-save tests passing

### Milestone 2 foundation gate (for current phase)
- [x] Runtime C ABI module (`VulkanAI`) loadable through loader
- [x] RenderHost adapter available for attach/resize/frame/present cycle
- [x] Runtime diagnostics + retry exposed in UI
- [x] Canvas foundation classes/panel integrated into project structure

---

## Recommended signoff command sequence

```bash
./scripts/build_with_prereqs.sh Debug on
cmake --build build --target format-check
cmake --build build --target lint
ctest --test-dir build --output-on-failure
```

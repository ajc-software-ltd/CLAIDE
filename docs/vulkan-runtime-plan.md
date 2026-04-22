# Vulkan Runtime Plan

This plan introduces `VulkanAI` as the Vulkan runtime module consumed by CLAIDE.

## Goals
- Encapsulate Vulkan-specific rendering logic in a shared module.
- Keep UI/app layers independent from direct Vulkan SDK usage.
- Provide diagnostics and fallback messaging when Vulkan is unavailable.

## Runtime artifacts
- Windows: `VulkanAI.dll`
- Linux: `libVulkanAI.so`

## Phase 1-4 scope
- Move Vulkan and GPU filter sources to `src/vulkan/`.
- Add public C ABI in `include/vulkanai/VulkanAI.h`.
- Build shared library target `VulkanAI` in CMake.
- Add app-side runtime loader (`Core::VulkanRuntimeLoader`).
- Expose surface lifecycle API stubs (`CreateSurface`, `BeginFrame`, `EndFrame`, `PresentFrame`).
- Surface fallback diagnostics in UI status/logs when runtime/module/Vulkan support is unavailable.
- Enforce runtime API compatibility handshake (`VulkanAI_GetApiVersion`).
- Expose capability query (`VulkanAI_GetCapabilities`) for diagnostics and compatibility views.
- Provide user-facing diagnostics via Help → Runtime Diagnostics.

## Fallback policy
- If runtime cannot initialize Vulkan, return explicit error code and message.
- Surface user guidance through `VulkanAI_GetInstallHelpUrl()`.

## Notes
- FFmpeg/miniaudio modules are intentionally deferred until their milestones.
- Vulkan runtime API remains stable while internals evolve.

## Remaining restructure scaffolding added
- `core/runtime/RenderHost.hpp` defines a render-host boundary for wx/native integration.
- `core/runtime/VulkanRenderHost.{hpp,cpp}` provides initial Vulkan runtime host adapter scaffold.
- `core/media/VideoFrameProvider.hpp` and `core/media/AudioProvider.hpp` define future ffmpeg/miniaudio module seams.
- `docs/branch-policy.md` documents main-only commit flow.
- `MainFrame` now drives a minimal runtime-host frame loop (`BeginFrame/EndFrame/Present`) and resize handling.

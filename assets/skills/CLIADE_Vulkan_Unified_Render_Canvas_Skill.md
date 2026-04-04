# CLIADE — Vulkan Unified Render Canvas Skill

AJC-Software Ltd © 2026

## Purpose

This file is a **project-specific OpenCode skill sheet** for **CLIADE**.
It is not a generic Vulkan note.
It exists to guide planning and implementation for:

- **Imaging Tools**
- **Photoshop-style implementation**
- **Vulkan-backed unified render canvas**

This skill must be used when designing, reviewing, or implementing the Vulkan canvas work for CLIADE.

---

## Repository Context

Current CLIADE state, as observed from the repo:

- Project is **C++23** with **wxWidgets 3.2+** UI and **ImageMagick** for image handling.
- Milestone 2 is **Image Processing**, already including `MediaService`, `ImageViewer`, file routing, and sidebar modes.
- The current image path uses a CPU-side `wxScrolledWindow` image viewer, not a GPU render canvas.
- The project architecture requires:
  - **UI in `src/ui/` only**
  - **Core logic in `src/core/`**
  - **RAII throughout**
  - **No unnecessary framework complexity**
  - **Commercially viable libraries only**

This skill therefore assumes the Vulkan work will **replace or sit alongside the current CPU image viewer** while preserving CLIADE architectural rules.

---

## What Milestone 2.1.1 Means in CLIADE

For this project, **Milestone 2.1.1** means:

> Introduce a **unified Vulkan render canvas** that becomes the long-term rendering surface for image editing tools, GPU-assisted preview, layered compositing, brush overlays, selection overlays, guides, transforms, and future media workflows.

It is **not** just “draw image with Vulkan”.
It must become the **foundation** for later Photoshop-style image tooling.

---

## Hard Project Constraints

OpenCode must respect these constraints for CLIADE:

1. **Do not break the current Milestone 2 app flow.**
   File open, media routing, docking, sidebar logic, and status-bar behavior must continue to work.

2. **Do not mix UI concerns with renderer internals.**
   wxWidgets event handling stays in UI classes.
   Vulkan device, swapchain, render graph, descriptors, pipelines, textures, and synchronization stay in renderer code.

3. **Do not make `MediaService` into a renderer.**
   `MediaService` remains responsible for type detection, metadata, and image data extraction.
   It may provide decoded pixel buffers, but it must not own Vulkan objects.

4. **Do not make the first Vulkan pass a full Photoshop clone.**
   Milestone 2.1.1 is the **render foundation**, not the full editor feature set.

5. **Prefer deterministic, debuggable architecture.**
   Correctness, validation, and maintainability come before flashy rendering.

---

## Recommended Milestone 2.1.1 Scope

### In Scope

- Vulkan initialization for a dockable CLIADE canvas
- Dedicated Vulkan canvas widget integrated with wxWidgets
- GPU texture upload for opened images
- Render pass or dynamic rendering path for 2D image presentation
- Pan and zoom in canvas space
- Checkerboard transparency background
- Basic overlay system for:
  - selection rectangles
  - guides
  - crop/transform bounds
  - brush cursor preview
- Resize-safe swapchain recreation
- Validation layer support in Debug
- Clean renderer shutdown
- Stable abstraction for future layers/tools

### Out of Scope for 2.1.1

- Full layer stack editing
- Adjustment layers
- Node graph composition
- Non-destructive history engine
- GPU compute brush engine
- Text engine
- Video/audio/3D Vulkan unification
- Multi-document Vulkan renderer sharing unless explicitly designed

---

## Recommended Architecture

### New Module Layout

Recommended new structure:

```text
src/
├── render/
│   ├── vulkan/
│   │   ├── VulkanContext.hpp/.cpp
│   │   ├── VulkanDevice.hpp/.cpp
│   │   ├── VulkanSwapchain.hpp/.cpp
│   │   ├── VulkanImage.hpp/.cpp
│   │   ├── VulkanBuffer.hpp/.cpp
│   │   ├── VulkanDescriptors.hpp/.cpp
│   │   ├── VulkanPipeline.hpp/.cpp
│   │   ├── VulkanRenderer.hpp/.cpp
│   │   ├── VulkanUploadContext.hpp/.cpp
│   │   ├── VulkanCanvasScene.hpp/.cpp
│   │   └── VulkanDebug.hpp/.cpp
│   └── canvas/
│       ├── CanvasDocument.hpp/.cpp
│       ├── CanvasViewState.hpp/.cpp
│       ├── CanvasOverlay.hpp/.cpp
│       └── CanvasGrid.hpp/.cpp
├── ui/
│   ├── VulkanCanvasPanel.hpp/.cpp
│   └── ImageViewer.hpp/.cpp   # retained temporarily or retired later
└── shaders/
    ├── canvas_image.vert
    ├── canvas_image.frag
    ├── canvas_overlay.vert
    ├── canvas_overlay.frag
    └── README.md
```

### Responsibility Split

#### `ui/VulkanCanvasPanel`
Owns:
- wxWidgets widget lifecycle
- resize events
- mouse input
- keyboard shortcuts
- focus handling
- forwarding input to canvas state
- requesting redraw

Does **not** own:
- raw Vulkan global state scattered through UI code
- shader compilation logic
- swapchain policy decisions outside renderer interface

#### `render/vulkan/*`
Owns:
- instance/device/surface/swapchain
- queues and command buffers
- descriptors
- pipelines
- staging uploads
- synchronization
- GPU images and samplers
- frame lifecycle

#### `render/canvas/*`
Owns:
- zoom/pan state
- canvas transform math
- image placement
- overlay data model
- checkerboard/grid rules
- future layer-friendly scene description

#### `core/MediaService`
Provides:
- decoded image bytes
- metadata
- format details

It may expose an API like:

```cpp
std::expected<DecodedImage, std::string> LoadDecodedImage(const std::filesystem::path& path);
```

Where `DecodedImage` contains width, height, format, stride, and pixel bytes.

---

## Preferred Integration Strategy

### Phase 1 — Parallel Integration

Do **not** immediately delete `ImageViewer`.
Instead:

- add `VulkanCanvasPanel`
- make image opening optionally route to it
- preserve current viewer until the Vulkan canvas is stable

This reduces risk and keeps Milestone 2 functional.

### Phase 2 — Promote Vulkan Canvas

Once stable:

- route image opens to Vulkan canvas by default
- retire or demote `ImageViewer`
- keep metadata/status flow intact

---

## Unified Render Canvas Design Rules

The unified canvas must be designed around a **document + view + renderer** split.

### Document State

Represents image/content state:
- image dimensions
- pixel format
- logical layer list later
- selection mask later
- edit state later

### View State

Represents how content is shown:
- zoom
- pan offset
- viewport size
- visible region
- grid visibility
- overlay toggles

### Renderer State

Represents GPU resources:
- swapchain images
- framebuffers or dynamic rendering attachments
- pipelines
- descriptor sets
- texture resources
- per-frame uniform data

This separation is mandatory.

---

## Vulkan Technical Direction

### API Style

Use one of these approaches consistently:

- **Vulkan-Hpp RAII wrappers** if the codebase wants safer C++ ownership
- or **C Vulkan API with strict RAII wrappers** if you want lower-level explicit control

Do **not** mix inconsistent ownership styles across the renderer.

### Recommended Initial Feature Set

For CLIADE canvas work, the first stable renderer should support:

- one graphics queue
- one present queue
- double or triple buffering
- sampled 2D textures for image display
- one pipeline for image draw
- one pipeline for overlay lines/quads
- push constants or small uniform buffer for view transform
- immutable samplers where practical

### Avoid Early Overengineering

Do not introduce in 2.1.1 unless clearly needed:
- bindless
- render graph framework
- ECS renderer integration
- complex allocator abstraction layers before basic stability
- premature multi-window device sharing

---

## wxWidgets + Vulkan Rules

Because CLIADE uses wxWidgets, OpenCode must:

1. Choose a platform-appropriate Vulkan presentation path compatible with wxWidgets native window handles.
2. Keep the Vulkan canvas as a normal CLIADE dockable panel.
3. Ensure resize handling is robust when panes are docked, floated, minimized, hidden, or restored.
4. Prevent redraw storms from excessive resize or paint events.
5. Ensure destruction order is safe when panes close or the app exits.

The panel must behave like a first-class CLIADE pane, not a special-case hack.

---

## Image Upload Pipeline

The canonical flow for opened images should be:

1. `MainFrame` routes image path to canvas view.
2. `MediaService` decodes image into CPU pixel buffer.
3. Renderer creates staging buffer.
4. Renderer uploads pixel data into device image.
5. Renderer transitions image layout properly.
6. Canvas scene references uploaded texture.
7. Draw path samples texture in fragment shader.

Do not load images directly inside random UI paint handlers.

---

## Minimum Shader Set for 2.1.1

### `canvas_image.vert`
Responsibilities:
- full-screen or quad vertex generation
- UV mapping
- view transform application if needed

### `canvas_image.frag`
Responsibilities:
- sample image texture
- composite over checkerboard for alpha images
- optional color management placeholder hooks

### `canvas_overlay.vert`
Responsibilities:
- transform overlay primitives into viewport space

### `canvas_overlay.frag`
Responsibilities:
- draw guides, selections, handles, crop bounds, cursor previews

---

## Shader Rules

1. Author shaders in **GLSL** first unless there is a strong project reason otherwise.
2. Compile to **SPIR-V** as part of the build.
3. Treat compiled shader artifacts as build outputs, not hand-maintained source.
4. Keep shader IO layouts explicit and stable.
5. Keep descriptor set layouts simple and documented.
6. No magic numbers for bindings.
7. One Markdown file in `src/shaders/README.md` should document:
   - each shader
   - descriptor bindings
   - vertex inputs
   - push constants
   - specialization constants

---

## C++ Rules for Vulkan in CLIADE

### Ownership
- Every Vulkan handle must have a clear owner.
- No leaking handles across unrelated classes.
- No raw lifetime ambiguity.

### Error Handling
- Surface initialization failures clearly to the UI.
- Log Vulkan setup failures with component context.
- Fail fast on unrecoverable renderer startup issues.

### Logging
Use structured component-prefixed logs such as:
- `VulkanContext: created instance`
- `VulkanDevice: selected physical device ...`
- `VulkanSwapchain: recreated 1920x1080`
- `VulkanRenderer: uploaded texture ...`

### Threading
For 2.1.1:
- keep rendering logic single-threaded unless a real need emerges
- async asset loading may be added later, but not as a hidden complication in the first canvas milestone

---

## Synchronization Rules

OpenCode must be extremely conservative and explicit here.

### Required Discipline
- No guessing on image layout transitions
- No silent implicit assumptions about availability/visibility
- Every upload path must define source and destination stages/access masks
- Every frame path must define acquire, render, and present sequencing clearly

### Initial Simplicity Target
Use a straightforward per-frame model:
- image available semaphore
- render finished semaphore
- in-flight fence

Only increase complexity when proven necessary.

---

## Texture and Format Rules

The Vulkan canvas must initially target reliable 8-bit workflows first.

### Initial support target
- `RGBA8` upload path
- optional conversion from decoded source formats into canonical renderer upload format

### Future-ready design
Keep the API extendable for:
- grayscale
- RGB
- RGBA
- 16-bit per channel
- float HDR
- linear vs sRGB handling

But do not block 2.1.1 trying to fully solve advanced color science.

---

## Status Bar Integration Rules

Current CLIADE already updates status bar information for image opens.
The Vulkan canvas path must preserve or improve that behavior.

At minimum, continue to expose:
- image dimensions
- format name
- filename

Later you may extend with:
- zoom level
- cursor coordinates
- sampled color
- selection size
- tool name

---

## Milestone 2.1.1 Deliverables

A Milestone 2.1.1 implementation is considered valid only if it delivers:

1. A new **Vulkan canvas panel** integrated into CLIADE docking.
2. Successful image display through Vulkan.
3. Pan and zoom working correctly.
4. Safe resize and swapchain recreation.
5. Debug validation layers enabled in Debug builds.
6. Clean shutdown without validation spam.
7. Shader build pipeline integrated into CMake.
8. Architecture that supports later overlays and tools.

---

## Suggested Increment Plan

### Step 1
Create renderer scaffolding only:
- instance
- surface
- physical device
- logical device
- swapchain
- frame loop clear color

### Step 2
Display a checkerboard-only canvas.

### Step 3
Upload and display a single image texture.

### Step 4
Add pan and zoom.

### Step 5
Add overlay pipeline.

### Step 6
Route CLIADE image opening path to Vulkan canvas.

### Step 7
Retain fallback path until stable.

---

## CMake Expectations

Add Vulkan support cleanly and explicitly.

### Required build behavior
- find Vulkan SDK
- compile GLSL shaders to SPIR-V
- copy or stage shaders into runtime output
- keep Debug and Release behavior predictable

### Do not
- bury shader compilation in fragile custom shell scripts only
- hardcode SDK paths in source files
- scatter Vulkan compile definitions everywhere

### Preferred additions
- `find_package(Vulkan REQUIRED)`
- custom target for shader compilation
- explicit dependency from app target to shader target

---

## File Naming Guidance

Use clear, stable names.

Good:
- `VulkanCanvasPanel.cpp`
- `VulkanRenderer.cpp`
- `CanvasViewState.hpp`
- `canvas_image.frag`

Bad:
- `RendererNew2.cpp`
- `vkstuff.hpp`
- `temp_shader.glsl`

---

## Testing Expectations

Milestone 2.1.1 should include at least:

### Unit-testable pieces
- canvas view transform math
- zoom limits
- pan clamping if implemented
- image metadata to texture descriptor translation helpers
- shader path resolution helpers

### Manual validation checklist
- app starts with validation layers enabled in Debug
- open PNG/JPEG/WebP/PSD path still works through CLIADE routing
- canvas resizes correctly when docked/floated
- zoom works without jitter
- pan works without desync
- closing app produces no Vulkan lifetime errors

---

## Migration Guidance From Current CLIADE State

Current repo behavior suggests this transition:

### Current
- `MainFrame` owns `ImageViewer`
- images are loaded into CPU memory and displayed via `wxBitmap`
- `MediaService` already knows media types and image metadata

### Target
- `MainFrame` owns `VulkanCanvasPanel`
- `MediaService` decodes pixels
- `VulkanRenderer` uploads pixels and renders image
- `CanvasViewState` controls zoom/pan/viewport transform
- overlays are GPU-rendered

This means the safest near-term change is:

- keep `OpenImage(path)` in `MainFrame`
- replace its display target from `ImageViewer` to `VulkanCanvasPanel`
- preserve metadata/status bar updates

---

## OpenCode Behaviour Rules

When working on CLIADE Vulkan tasks, OpenCode must:

1. Read `AGENTS.md` first.
2. Respect CLIADE architecture boundaries.
3. Avoid rewriting unrelated Milestone 2 systems.
4. Keep patches focused and minimal.
5. Explain Vulkan synchronization and lifetime decisions explicitly.
6. Prefer small validated steps over large speculative rewrites.
7. Never claim Vulkan code is correct without checking destruction order, resize handling, and synchronization.

---

## Response Format for CLIADE Vulkan Tasks

Use this structure unless the user requests otherwise:

### ANALYSIS
What part of the unified render canvas is being worked on and what existing CLIADE code it touches.

### PLAN
Exactly what files/classes will be added or changed.

### CHANGES
Concrete file list and architectural effect.

### CODE
Patch or implementation.

### VALIDATION
How the Vulkan path was checked:
- compile
- shader build
- runtime startup
- resize
- open image
- shutdown

### RISKS
Known limitations, deferred items, or future follow-ups.

---

## What OpenCode Should Not Do

- Do not turn `MainFrame.cpp` into a giant Vulkan implementation file.
- Do not decode image bytes in shaders.
- Do not make `MediaService` own GPU handles.
- Do not collapse document/view/renderer state into one class.
- Do not skip validation layer support in Debug.
- Do not introduce “temporary” synchronization shortcuts that are actually undefined behavior.
- Do not remove the current image path until the Vulkan path is stable.

---

## Ideal First PR / First Patch Goal

The best first 2.1.1 patch is:

> A dockable `VulkanCanvasPanel` that initializes Vulkan safely, renders a checkerboard or solid test canvas, survives resize, and shuts down cleanly.

That patch proves the canvas foundation.
A second patch can add image texture upload.

---

## Final Instruction

For CLIADE Milestone 2.1.1, treat Vulkan as the **long-term rendering foundation** for the image editor, not a cosmetic side experiment.

The correct approach is:
- stable renderer core first
- image presentation second
- overlays third
- editing tools afterwards

Everything must support the future **Photoshop-style unified render canvas** vision while remaining consistent with current CLIADE architecture.

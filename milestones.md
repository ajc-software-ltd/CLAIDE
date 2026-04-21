# CLIADE — Milestone Roadmap

AJC-Software Ltd © 2026

AI-powered AIO IDE for code, media creation, and content generation.

---

## Checkpoint System

After each milestone is complete:
1. `git add -A`
2. `git commit -m "Milestone N: <description>"`
3. `git tag v0.0.<commit_count>-dev`
4. `git push origin main --tags`

Current versioning baseline: `v0.0.56-dev` (commit-count based).

| Milestone | Status |
|-----------|--------|
| 1: Core IDE Shell | ✅ Complete |
| 2: Vulkan Canvas (Universal Render Layer) | 🚧 In Progress (foundation landed) |
| 3: Text Enhancement | ⏳ Planned |
| 4: Image Processing (AVIM: Image) | ⏳ Planned |
| 5: Video Player (AVIM: Video) | ⏳ Planned |
| 6: 3D Model Viewer (AVIM: Model) | ⏳ Planned |
| 7: Unified Workspace | ⏳ Planned |
| 8: AI Integration Layer | ⏳ Planned |
| 9: AI Content Generation | ⏳ Planned |

---

## Milestone 1: Core IDE Shell ✅

**Tag:** `v0.0.1-dev`
**Status:** Complete (subject to evolution)

### Deliverables
- wxAui docking system with floating, pinning, resizable panels
- Custom IconBar sidebar with folder navigation
- Tabbed text editor with per-tab Document tracking
- Dark theme with custom background image
- UTF-8, UTF-16 LE/BE encoding detection and conversion
- Safe save via temp file + atomic replace
- spdlog logging with rotating file sink, wxLog routing, assert capture
- Prompt bar with Send/Clear buttons
- Core unit test suite (Encoding, FileService, Document and expanded modules) via Catch2/CTest
- clang-format, clang-tidy, sanitizer support via CMake targets
- Git repo initialized, initial commit pushed to private GitHub

### New Files
| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point |
| `src/app/Application.{hpp,cpp}` | wxApp bootstrap, spdlog, wxLog routing, assert handler |
| `src/core/Encoding.{hpp,cpp}` | BOM detection, UTF-8/16 encode/decode |
| `src/core/Document.{hpp,cpp}` | Document model, dirty tracking |
| `src/core/FileService.{hpp,cpp}` | Safe save, load, delete |
| `src/ui/Theme.{hpp,cpp}` | Dark theme colours |
| `src/ui/EditorPanel.{hpp,cpp}` | wxTextCtrl editor widget |
| `src/ui/MainFrame.{hpp,cpp}` | Main window: AUI, IconBar, Explorer, PromptBar, tabs |
| `src/platform/PlatformPaths.{hpp,cpp}` | Cross-platform path helpers |
| `tests/EncodingTests.cpp` | 16 encoding tests |
| `tests/FileServiceTests.cpp` | 10 file service tests |
| `tests/DocumentTests.cpp` | 10 document tests |

### Dependencies
- wxWidgets 3.2+ (wxWindows License)
- spdlog (MIT)
- Catch2 v3 (BSL-1.0)

---

## Milestone 2: Vulkan Canvas (Universal Render Layer)

**Tag:** `v0.0.2-dev`
**Status:** In Progress (runtime foundation integrated)

### Vision
The Vulkan Canvas is the **universal render surface** for all content types in CLIADE. Every piece of content — text, images, video, 3D models — renders through the Canvas. The AI uses Canvas instances programmatically to create, composite, and output content.

### Deliverables
- `Canvas` — Universal canvas interface with CPU+GPU dual-path rendering
- `CanvasDocument` — Content state (text, images, video frames, 3D meshes)
- `CanvasView` — View state (zoom, pan, viewport transform)
- `VulkanContext` — Vulkan instance, device, queues, validation layers
- `VulkanSwapchain` — Surface, swapchain, resize-safe recreation
- `VulkanRenderer` — Render loop, pipelines, frame lifecycle
- `VulkanTexture` — GPU image upload from decoded pixel buffers
- `VulkanText` — Glyph atlas, text rendering pipeline
- `CPURenderer` — Software fallback (identical output to GPU)
- `CanvasPanel` — wxWidgets dockable panel wrapper
- Multi-canvas management (AI can create/manage multiple canvases)
- Picture-in-Picture compositing (layered canvases)
- Pan/zoom in canvas space
- Checkerboard transparency background
- Debug validation layers in Debug builds
- Clean shutdown without validation errors

### Architecture
```
src/
├── render/
│   ├── Canvas.{hpp,cpp}              # Universal canvas interface
│   ├── CanvasDocument.{hpp,cpp}      # Content state
│   ├── CanvasView.{hpp,cpp}          # View state
│   ├── vulkan/
│   │   ├── VulkanContext.{hpp,cpp}   # Instance, device, queues
│   │   ├── VulkanSwapchain.{hpp,cpp} # Surface, swapchain, resize
│   │   ├── VulkanRenderer.{hpp,cpp}  # Render loop, pipelines
│   │   ├── VulkanTexture.{hpp,cpp}   # GPU image upload
│   │   ├── VulkanText.{hpp,cpp}      # Text rendering
│   │   └── VulkanDebug.{hpp,cpp}     # Validation, debug markers
│   └── cpu/
│       └── CPURenderer.{hpp,cpp}     # Software fallback
├── ui/
│   └── CanvasPanel.{hpp,cpp}         # wxWidgets dockable wrapper
└── shaders/
    ├── canvas_text.vert/frag
    ├── canvas_image.vert/frag
    └── canvas_overlay.vert/frag
```

### AI Integration Points
- `Canvas::Create()` — AI creates new canvas instances
- `Canvas::Render(content)` — AI renders content to canvas
- `Canvas::Composite(canvases)` — AI composites multiple canvases (PiP)
- `Canvas::Export(path, format)` — AI exports canvas to file
- `Canvas::GetPixelData()` — AI extracts pixel buffer for analysis

### Dependencies Added
- Vulkan SDK (Apache 2.0)
- glslang (Apache 2.0, shader compilation)
- SPIRV-Tools (Apache 2.0, shader optimization)

### Checkpoint
- Auto-commit with tag `v0.0.2-dev`

---

## Milestone 3: Text Enhancement

**Tag:** `v0.0.3-dev`
**Status:** Planned

### Deliverables
- Rich text formatting (bold, italic, underline, fonts, colors, sizes)
- Paragraph formatting (alignment, indentation, spacing)
- Lists (bulleted, numbered)
- Tables
- Headers/footers
- Page layout (margins, orientation, size)
- Format support: DOCX, PDF, RTF, ODT, TXT, MD
- Format conversion between types
- AI-accessible document model (structured content)
- Text rendering through Vulkan Canvas

### Dependencies Added
- libzip (BSD, DOCX parsing)
- poppler (GPL, PDF rendering) or MuPDF (AGPL)

### Checkpoint
- Auto-commit with tag `v0.0.3-dev`

---

## Milestone 4: Image Processing (AVIM: Image)

**Tag:** `v0.0.4-dev`
**Status:** Planned

### Deliverables
- GPU-accelerated image filters (brightness, contrast, blur, sharpen, etc.)
- Image editing tools (crop, resize, rotate, flip)
- Color adjustment tools (levels, curves, hue, saturation)
- Layer support (basic compositing)
- Format support: PNG, JPEG, BMP, GIF, TIFF, WebP, PSD, PSB
- AI-accessible image operations
- Image rendering through Vulkan Canvas

### Dependencies Added
- None (reuses Vulkan Canvas from M2)

### Checkpoint
- Auto-commit with tag `v0.0.4-dev`

---

## Milestone 5: Video Player (AVIM: Video)

**Tag:** `v0.0.5-dev`
**Status:** Planned

### Deliverables
- Video playback via libmpv
- Audio playback via libmpv
- Controls: Play/Pause, Stop, scrub bar, volume, time display
- Frame extraction for AI analysis
- Codec support: MP4, WebM, MKV, AVI, MOV (via libmpv)
- Video rendering through Vulkan Canvas
- AI-ready: `ExtractFrame(path, timestamp)` → image bytes

### Dependencies Added
- libmpv (LGPL 2.1+, dynamic link)

### Checkpoint
- Auto-commit with tag `v0.0.5-dev`

---

## Milestone 6: 3D Model Viewer (AVIM: Model)

**Tag:** `v0.0.6-dev`
**Status:** Planned

### Deliverables
- 3D model loading via Assimp (FBX, OBJ, glTF, GLB)
- Vulkan rendering pipeline for geometry
- Orbit camera: left-drag rotate, right-drag pan, scroll zoom
- Texture mapping, basic lighting
- Animation playback support
- Status bar: vertex count, face count, texture count
- AI-ready: `GetModelMetadata(path)` → mesh/texture/bone info
- 3D rendering through Vulkan Canvas

### Dependencies Added
- Assimp (BSD 3-Clause)

### Checkpoint
- Auto-commit with tag `v0.0.6-dev`

---

## Milestone 7: Unified Workspace

**Tag:** `v0.0.7-dev`
**Status:** Planned

### Deliverables
- Split workspace: code editor + Canvas side by side
- Multi-canvas layout management
- AI chat panel docked alongside
- Project-level asset browser
- Consistent theme across all components
- Keyboard shortcuts for AI actions
- Python scripting: full IDE automation via Python
- Settings/preferences panel
- Project configuration files

### Checkpoint
- Auto-commit with tag `v0.0.7-dev`

---

## Milestone 8: AI Integration Layer

**Tag:** `v0.0.8-dev`
**Status:** Planned

### Deliverables
- `AIInterface` — AI provider abstraction (GPT, Claude, local models)
- `MediaExtractor` — Frame/image/audio/model data extraction
- `AIPanel` — Chat/response panel, dockable
- Context sharing: AI sees open files, canvases, models simultaneously
- Python: full pybind11 bindings for all components
- AI can create/manage Canvas instances programmatically

### Dependencies Added
- cURL (for AI API calls) or httplib (header-only)

### Checkpoint
- Auto-commit with tag `v0.0.8-dev`

---

## Milestone 9: AI Content Generation

**Tag:** `v0.0.9-dev`
**Status:** Planned

### Deliverables
- `GenerationPanel` — Prompt input, generation settings, output gallery
- `GenerationService` — Orchestrates image/video/model generation APIs
- Text-to-image generation (DALL-E, Stable Diffusion, etc.)
- Image-to-image generation (reference images from Canvas)
- Text-to-video generation
- Image/Text-to-3D model generation
- Generated content appears as Canvas layers automatically
- Generation history and versioning
- Python: generation scripting support

### Checkpoint
- Auto-commit with tag `v0.0.9-dev`

---

## Architecture Principles

1. **Vulkan Canvas is the universal render layer** — all content renders through it
2. **CPU+GPU dual-path** — Vulkan when available, CPU fallback always functional
3. **Multi-canvas support** — AI can create/manage multiple canvases
4. **Picture-in-Picture compositing** — Canvases can be layered
5. **AI-accessible APIs** — All Canvas operations exposed programmatically
6. **No dead code** — every component serves the AIO vision
7. **Commercial-free libraries only** — verified licenses throughout

## Library License Verification

All libraries used in this project are commercially viable at zero cost:

| Library | License | Commercial Use | Notes |
|---------|---------|---------------|-------|
| wxWidgets | wxWindows Library License | ✅ Yes | LGPL with exception |
| spdlog | MIT | ✅ Yes | No restrictions |
| Catch2 | BSL-1.0 | ✅ Yes | Test-only, not shipped |
| ImageMagick | ImageMagick License (Apache-like) | ✅ Yes | Free for commercial use |
| libmpv | LGPL 2.1+ | ✅ Yes | Must dynamically link |
| Assimp | BSD 3-Clause | ✅ Yes | Attribution only |
| Vulkan SDK | Apache 2.0 | ✅ Yes | Free for commercial use |
| pybind11 | BSD 3-Clause | ✅ Yes | Header-only |
| cURL | MIT/X derivative | ✅ Yes | Free for commercial use |
| glslang | Apache 2.0 | ✅ Yes | Shader compilation |
| SPIRV-Tools | Apache 2.0 | ✅ Yes | Shader optimization |

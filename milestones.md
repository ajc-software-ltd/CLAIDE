# CLIADE — Milestone Roadmap

AJC-Software Ltd © 2026

AI-powered AIO IDE for code, media creation, and content generation.

---

## Checkpoint System

After each milestone is complete:
1. `git add -A`
2. `git commit -m "Milestone N: <description>"`
3. `git tag v0.0.<N>-dev`
4. `git push origin main --tags`

| Milestone | Tag | Status |
|-----------|-----|--------|
| 1: Core IDE Shell | `v0.0.1-dev` | ✅ Complete |
| 2: Image Processing | `v0.0.3-dev` | ⏳ In Progress |
| 3: Video Player | `v0.0.3-dev` | ⏳ Planned |
| 4: Audio Player | `v0.0.4-dev` | ⏳ Planned |
| 5: 3D Model Viewer | `v0.0.5-dev` | ⏳ Planned |
| 6: AI Integration Layer | `v0.0.6-dev` | ⏳ Planned |
| 7: AI Content Generation | `v0.0.7-dev` | ⏳ Planned |
| 8: Unified Workspace | `v0.0.8-dev` | ⏳ Planned |

---

## Milestone 1: Core IDE Shell ✅

**Tag:** `v0.0.1-dev`
**Status:** Complete

### Deliverables
- wxAui docking system with floating, pinning, resizable panels
- Custom IconBar sidebar with folder navigation
- Tabbed text editor with per-tab Document tracking
- Dark theme with custom background image
- UTF-8, UTF-16 LE/BE encoding detection and conversion
- Safe save via temp file + atomic replace
- spdlog logging with rotating file sink, wxLog routing, assert capture
- Prompt bar with Send/Clear buttons
- 36 unit tests (Encoding, FileService, Document) via Catch2/CTest
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

## Milestone 2: Image Processing (AVIM: Image)

**Tag:** `v0.0.3-dev`
**Status:** ⏳ In Progress

### Deliverables
- `MediaService` — unified media type detection, metadata extraction, AI-ready extraction stubs
- `ImageViewer` — `wxScrolledWindow` with zoom (mouse wheel, cursor-centered), pan (drag), fit-to-window, actual size
- `File > Open` detects all media types and routes to correct viewer
- Status bar: dimensions, file size, color depth, format name
- IconBar sidebar with 4 icons: Files, Images, Video, 3D Models
- File explorer with file selection support (double-click to open)
- Sidebar mode controls sidebar panels only, center pane persists across mode changes
- Python foundation: pybind11 integration, Python console panel (stub)

### Supported Formats
- **wxImage native:** PNG, JPEG, BMP, GIF, TIFF, ICO, PCX, PNM, XPM
- **ImageMagick:** PSD (layered), PSB (large format), WebP

### New Files
| File | Purpose |
|------|---------|
| `src/core/MediaService.{hpp,cpp}` | Media type detection, metadata, AI extraction stubs |
| `src/ui/ImageViewer.{hpp,cpp}` | Image viewer with zoom, pan, fit, context menu |
| `src/ui/PythonConsole.{hpp,cpp}` | Python REPL panel (stub) |

### Dependencies Added
- ImageMagick 7 (Apache-like license)
- pybind11 (BSD 3-Clause, header-only via FetchContent)

### Checkpoint
- Auto-commit with tag `v0.0.3-dev`
- Python scripting foundation in place for future milestones

---

## Milestone 3: Video Player (AVIM: Video)

**Tag:** `v0.0.3-dev`
**Status:** Planned

### Deliverables
- `VideoPlayer` — libmpv wrapper with embedded playback
- Controls bar: Play/Pause, Stop, scrub bar (seek), volume slider, time display
- MP4, WebM, MKV, AVI, MOV (full codec support via libmpv)
- AI-ready: `MediaService::ExtractFrame(path, timestamp)` → image bytes
- Python: `pybind11` bindings for video control from scripts

### New Files
| File | Purpose |
|------|---------|
| `src/ui/VideoPlayer.{hpp,cpp}` | libmpv video player with controls |
| `src/ai/MediaExtractor.{hpp,cpp}` | Frame extraction for AI (stub) |

### Dependencies Added
- libmpv (LGPL 2.1+, dynamic link)

### Checkpoint
- Auto-commit with tag `v0.0.3-dev`

---

## Milestone 4: Audio Player (AVIM: Audio)

**Tag:** `v0.0.4-dev`
**Status:** Planned

### Deliverables
- `AudioPlayer` — libmpv audio with waveform visualization
- Controls: Play/Pause, Stop, volume, scrub bar
- MP3, WAV, OGG, FLAC
- Waveform amplitude visualization (custom wxPanel)
- AI-ready: `MediaService::GetAudioWaveform(path)` → amplitude array
- Python: audio playback and waveform access from scripts

### New Files
| File | Purpose |
|------|---------|
| `src/ui/AudioPlayer.{hpp,cpp}` | Audio player with waveform |

### Dependencies Added
- None (reuses libmpv from Milestone 3)

### Checkpoint
- Auto-commit with tag `v0.0.4-dev`

---

## Milestone 5: 3D Model Viewer (AVIM: Model)

**Tag:** `v0.0.5-dev`
**Status:** Planned

### Deliverables
- `ModelViewer` — `wxGLCanvas` with Assimp loader
- FBX, OBJ, glTF, GLB loading
- OpenGL rendering with basic Phong lighting
- Texture mapping support
- Orbit camera: left-drag rotate, right-drag pan, scroll zoom
- Status bar: vertex count, face count, texture count
- AI-ready: `MediaService::GetModelMetadata(path)` → mesh/texture/bone info
- Python: 3D model inspection from scripts

### New Files
| File | Purpose |
|------|---------|
| `src/ui/ModelViewer.{hpp,cpp}` | 3D model viewer with OpenGL |

### Dependencies Added
- Assimp (BSD 3-Clause)
- OpenGL (Khronos, free)

### Checkpoint
- Auto-commit with tag `v0.0.5-dev`

---

## Milestone 6: AI Integration Layer

**Tag:** `v0.0.6-dev`
**Status:** Planned

### Deliverables
- `AIInterface` — AI provider abstraction (GPT, Claude, local models)
- `MediaExtractor` — full implementation: frame/image/audio/model data extraction
- `AIPanel` — chat/response panel, dockable
- Python: full pybind11 bindings for all AVIM components
- Context sharing: AI sees open files, images, models simultaneously

### New Files
| File | Purpose |
|------|---------|
| `src/ai/AIInterface.{hpp,cpp}` | AI provider abstraction |
| `src/ai/MediaExtractor.{hpp,cpp}` | Media data extraction for AI |
| `src/ui/AIPanel.{hpp,cpp}` | AI chat/response panel |
| `src/python/Bindings.cpp` | pybind11 module definition |

### Dependencies Added
- cURL (for AI API calls) or httplib (header-only)

### Checkpoint
- Auto-commit with tag `v0.0.6-dev`

---

## Milestone 7: AI Content Generation

**Tag:** `v0.0.7-dev`
**Status:** Planned

### Deliverables
- `GenerationPanel` — prompt input, generation settings, output gallery
- `GenerationService` — orchestrates image/video/model generation APIs
- Text-to-image generation (DALL-E, Stable Diffusion, etc.)
- Image-to-image generation (reference images from AVIM tabs)
- Text-to-video generation
- Image/Text-to-3D model generation
- Generated content appears as new AVIM tabs automatically
- Generation history and versioning
- Python: generation scripting support

### New Files
| File | Purpose |
|------|---------|
| `src/ui/GenerationPanel.{hpp,cpp}` | Generation UI |
| `src/ai/GenerationService.{hpp,cpp}` | Generation orchestration |

### Checkpoint
- Auto-commit with tag `v0.0.7-dev`

---

## Milestone 8: Unified Workspace (AIO IDE)

**Tag:** `v0.0.8-dev`
**Status:** Planned

### Deliverables
- Split workspace: code editor + media viewer side by side
- AI chat panel docked alongside
- Project-level media management (asset browser)
- Consistent theme across all components
- Keyboard shortcuts for AI actions (Ctrl+Shift+G for generation)
- Python scripting: full IDE automation via Python
- Settings/preferences panel
- Project configuration files

### New Files
| File | Purpose |
|------|---------|
| `src/ui/AssetBrowser.{hpp,cpp}` | Project media management |
| `src/ui/SettingsPanel.{hpp,cpp}` | Preferences |
| `src/python/IDEBindings.cpp` | Full IDE automation bindings |

### Checkpoint
- Auto-commit with tag `v0.0.8-dev`

---

## Architecture Principles

1. **Every viewer is a `wxPanel`** — dockable, tabbable, consistent behavior
2. **`MediaService` is the single source of truth** — type detection, metadata, AI extraction
3. **AI modules are provider-agnostic** — swap GPT/Claude/local without touching UI
4. **Python scripting at every layer** — users can automate any AVIM component
5. **No dead code** — every component serves the AIO vision
6. **Commercial-free libraries only** — verified licenses throughout

## Python Scripting Scope

Python is embedded at every milestone with increasing capability:

| Milestone | Python Capability |
|-----------|------------------|
| 2 | Console panel (stub), pybind11 foundation |
| 3 | Video control from scripts (play, seek, extract frame) |
| 4 | Audio control + waveform access |
| 5 | 3D model inspection (mesh data, camera control) |
| 6 | Full AI provider scripting, media extraction pipeline |
| 7 | Generation scripting (create images, video, models from Python) |
| 8 | Full IDE automation (open files, manipulate UI, run builds) |

**Binding approach:** pybind11 (BSD 3-Clause, header-only, CMake FetchContent)

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
| OpenGL | Khronos Standard | ✅ Yes | Free, no license cost |
| pybind11 | BSD 3-Clause | ✅ Yes | Header-only |
| cURL | MIT/X derivative | ✅ Yes | Free for commercial use |
| httplib | BSD 3-Clause | ✅ Yes | Header-only alternative to cURL |

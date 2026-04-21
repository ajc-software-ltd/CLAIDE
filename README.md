# CLAIDE AI Content Creator

A cross-platform C++23 AIO IDE for code, media creation, and AI-powered content generation.

**AJC-Software Ltd © 2026**

---

## Versioning

- Current build version: **`0.0.112-dev`**
- Tag format from this point forward: **`v0.0.<commit_count>-dev`**
- Example for current state: **`v0.0.112-dev`**

## Repository Rename Readiness

- Post-rename checklist: `docs/repo-rename-readiness.md`

---

## Features

- **Dark theme** by default
- **UTF-8** internal text representation
- **UTF-8 BOM**, **UTF-16 LE/BE** detection and conversion
- **Safe save** via temporary file + atomic replace
- **CRUD** operations: New, Open, Save, Save As, Delete
- **Undo/Redo**, Copy/Cut/Paste, Select All
- **Keyboard shortcuts** on Linux and Windows
- **spdlog** logging with rotating file output
- **Catch2** unit tests for core modules
- **wxAui** docking system with floating, pinning, and resizable panels
- **Custom icon bar** sidebar with folder navigation
- **Tabbed editor** with per-tab document tracking
- **Prompt bar** with Send/Clear buttons

---

## Current Milestone System

The roadmap is currently being executed in milestone phases focused on core stability first.

| Milestone | Focus | Status |
|-----------|-------|--------|
| **Milestone 1** | Core IDE Shell | ✅ Complete |
| **Milestone 2** | Vulkan Canvas (universal render layer) | 🚧 In Progress (foundation landed) |
| **Milestone 3** | Text enhancement | ⏳ Planned |
| **Milestone 4** | Image processing | ⏳ Planned |
| **Milestone 5** | Video player | ⏳ Planned |
| **Milestone 6** | 3D model viewer | ⏳ Planned |
| **Milestone 7** | Unified workspace | ⏳ Planned |
| **Milestone 8** | AI integration layer | ⏳ Planned |
| **Milestone 9** | AI content generation | ⏳ Planned |

### Current stream status (at this time)
- ✅ **Core IDE Shell** is complete (Milestone 1).
- 🚧 **Vulkan Canvas** foundation is landed and active development is in progress (Milestone 2).
- ⏳ Feature tracks (text/image/video/3D/AI) build on the unified render surface roadmap.

See `milestones.md` for the canonical milestone breakdown and checkpoint workflow.

## Current Implementation Scope

### Functional now
- Text editor workflow (new/open/edit tabs) with document state tracking
- Image loading/viewing workflow with metadata display
- File type routing for text/image/video/audio/model extensions
- Core services: encoding, file IO, media type detection, document state, mipmap/tile cache utilities
- Unit test suite for core modules via Catch2/CTest

### Planned or placeholder
- Full Vulkan canvas/render-layer implementation completion (Milestone 2 in progress)
- Full image-processing milestone work (Milestone 3+)
- Full video/audio playback UI and controls
- 3D model rendering pipeline
- AI chat/provider integration panels and generation workflows
- Full Vulkan compute/render path replacing CPU fallback for image operations

---

## Prerequisites

### Linux (CachyOS / Arch)

```bash
sudo pacman -S wxwidgets-gtk3 spdlog catch2 cmake clang
```

### Windows

Install via vcpkg or package manager:
- wxWidgets 3.2+
- spdlog
- Catch2 v3
- CMake
- MSVC or MinGW

## Building

```bash
# REQUIRED FIRST STEP on Ubuntu/Debian:
# installs and verifies wxWidgets, Vulkan headers/tools, ImageMagick++, spdlog, Catch2
./scripts/build_with_prereqs.sh Debug on off on

# or (same first-time flow)
make dev-build-first-time

# Day-to-day build (skip prereq bootstrap)
make dev-build

# Configure (recommended preset for LSP/clangd compatibility)
cmake --preset dev  # primary fast preset (Ninja)

# Or manual configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug  # manual fallback

# Build
cmake --build build

# Run
./build/CLAIDE
```

### Release Build

```bash
# Bootstrap+verify dependencies, then build release
./scripts/build_with_prereqs.sh Release off off on

# or
make dev-release

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### With Sanitizers (Debug)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build
```


- `dev` preset uses **Ninja** as the primary fast path for local/Codex workflows.
- `dev-make` preset is a fallback when Ninja is unavailable.
- If configuration fails due to missing tools/libraries, run `make bootstrap` (or `./scripts/bootstrap_prereqs_ubuntu.sh`) then re-run configure/build.
- `ccache` is enabled in `dev`/`dev-make` presets; use `ccache -s` to monitor cache hit rates.

## Testing

```bash
# First-time setup only
make bootstrap

# Run tests (normal day-to-day)
make dev-test

# Run all tests
ctest --test-dir build --output-on-failure

# Run a single test
ctest --test-dir build -R Encoding --output-on-failure
```

## Code Quality

```bash
# Format source files
cmake --build build --target format

# Check formatting
cmake --build build --target format-check

# Fast local static analysis (changed files only)
cmake --build build --target lint-fast

# Full static analysis
cmake --build build --target lint

# Full validation (format + lint + test)
cmake --build build --target validate
```

> Note: `lint` intentionally disables `modernize-use-std-print` due to an upstream clang-tidy stability issue observed on this project.
> Lint reporting now writes both raw and deduplicated reports under `build/reports/`; prioritize unique diagnostics from `clang-tidy.summary.md`.
> Test lint also suppresses `bugprone-chained-comparison` because Catch2 assertion decomposition generates high-volume false positives.

## Release Management

- Milestone roadmap and status source: `milestones.md`
- Release traceability map (milestone → commit → tag): `RELEASES.md`
- Operational release checklist: `docs/release-checklist.md`
- Branch/merge workflow: `docs/branch-policy.md`
- Required merge validation gates: `build_with_prereqs` + `format-check` + `lint` + full `ctest`

## Project Structure

```
CLAIDE/
├── CMakeLists.txt                    # C++23, wxWidgets/spdlog/Catch2, sanitizers, format/lint/test/validate targets
├── AGENTS.md                         # Agent instructions and code standards
├── milestones.md                     # 8-milestone roadmap with checkpoint system
├── README.md                         # This file
├── .gitignore                        # Git ignore rules
├── .clang-format                     # LLVM base, 4-space indent, 120 column limit
├── .clang-tidy                       # cppcoreguidelines/modernize/readability/bugprone/performance checks
├── resources.rc                      # Windows application icon resource
├── include/
│   └── vulkanai/
│       └── VulkanAI.h                # Public C ABI for Vulkan runtime module
├── src/
│   ├── main.cpp                      # Entry point (includes Application.hpp)
│   ├── app/
│   │   ├── Application.hpp           # wxApp subclass declaration
│   │   └── Application.cpp           # Bootstrap, spdlog init (rotating file + console), wxLog routing, assert handler, PNG handler registration
│   ├── core/
│   │   ├── Encoding.hpp              # BOM detection, UTF-8/UTF-16 LE/BE encode/decode API
│   │   ├── Encoding.cpp              # Full encoding implementation with surrogate pair support, UTF-8 validation
│   │   ├── Document.hpp              # Document model: content, path, dirty tracking, encoding metadata
│   │   ├── Document.cpp              # Document implementation with display name generation
│   │   ├── FileService.hpp           # Safe save, load, delete API
│   │   └── FileService.cpp           # Atomic replace via temp file, encoding-aware IO, returns DecodeResult with metadata
│   ├── ui/
│   │   ├── Theme.hpp                 # Dark theme colour palette struct
│   │   ├── Theme.cpp                 # Dark colours + Windows remap option
│   │   ├── EditorPanel.hpp           # wxTextCtrl subclass for editor
│   │   ├── EditorPanel.cpp           # Monospace font, dark colours, multi-line editor
│   │   └── MainFrame.hpp/cpp         # Main window: wxAuiManager, IconBar, FileExplorerPanel, PromptBar, BackgroundPanel, tabbed editors, per-tab Document tracking
│   ├── platform/
│       ├── PlatformPaths.hpp         # Cross-platform app data/log/project root path helpers
│       └── PlatformPaths.cpp         # XDG on Linux, APPDATA on Windows, environment/cwd-based root resolution
│   └── vulkan/
│       ├── api/VulkanAI.cpp          # Vulkan runtime shared-library API entry points
│       ├── runtime/VulkanContext.*   # Vulkan instance/device/queue/bootstrap
│       ├── runtime/VulkanValidation.*# Validation layer and extension helpers
│       ├── filters/GPUEngine.*       # GPU filter execution + CPU fallback logic
│       └── shaders/*.comp            # Vulkan compute shaders
├── tests/
│   ├── EncodingTests.cpp             # 16 tests: BOM detection, decode, encode, round-trip, invalid data
│   ├── FileServiceTests.cpp          # 10 tests: load, save (UTF-8/BOM/UTF-16), delete, overwrite, existence
│   └── DocumentTests.cpp             # 10 tests: lifecycle, dirty tracking, encoding, clear, display name
└── assets/
    ├── canvas.png                    # Application background image
    ├── icons/
    │   ├── folder_icon_128x128.png   # Sidebar folder icon
    │   ├── send_icon.png             # Prompt bar send button icon
    │   ├── clear_icon.png            # Prompt bar clear button icon
    │   ├── app_icon.png              # Application window/taskbar icon
    │   ├── media_play_icon.png       # Media control play icon
    │   ├── media_eject_icon.png      # Media control eject icon
    │   ├── media_rewind_icon.png     # Media control rewind icon
    │   ├── media_fast_forward_icon.png # Media control fast-forward icon
    │   ├── media_pause_icon.png      # Media control pause icon
    │   ├── media_stop_icon.png       # Media control stop icon
    │   └── media_record_icon.png     # Media control record icon
    └── skills/
        └── wxwidgets-reference.md    # wxWidgets reference extracted from PDF
```

## Architecture

- **UI** (`src/ui/`) — Presentation only; no disk IO
- **Core** (`src/core/`) — Document, Encoding, FileService
- **App** (`src/app/`) — Bootstrap, lifecycle, logging init
- **Platform** (`src/platform/`) — OS-specific paths/config
- **Vulkan Runtime** (`src/vulkan/` + `include/vulkanai/`) — shared rendering module boundary (`VulkanAI.dll` / `libVulkanAI.so`)

## License

AJC-Software Ltd © 2026


### clangd / OpenCode / LM Studio LSP troubleshooting

- Configure with `cmake --preset dev  # primary fast preset (Ninja)` so `build/compile_commands.json` is generated.
- Sync compile database to repo root for language servers: `cmake --build build --target refresh-compile-commands`.
- Restart the language server if diagnostics do not appear after reconfigure.
- `.clangd`, `.clang-tidy`, and `.clang-format` in repo root define diagnostics/format behavior.

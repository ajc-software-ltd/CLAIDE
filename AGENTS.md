# AGENTS.md — CLIADE AI Content Creator

AJC-Software Ltd © 2026

AI-powered AIO IDE for code, media creation, and content generation (Linux x64 / Windows x64) using wxWidgets 3.2+.

---

## Codex Processing Order (Required)

For Codex-based agents, process project instructions in this order:

1. Read `codex.md` and apply its practices first.
2. Continue with the rest of this `AGENTS.md`.
3. If instructions conflict, prefer direct system/developer/user instructions, then `codex.md`, then `AGENTS.md`.

The intent is to keep changes simple, surgical, and verifiable while preserving CLIADE architecture constraints.

---

## Build / Lint / Test Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug        # or Release
cmake --build build

# Run a single test
ctest --test-dir build -R <test_name_pattern> --output-on-failure

# Run all tests
ctest --test-dir build --output-on-failure

# Format all source files
cmake --build build --target format

# Check formatting without modifying
cmake --build build --target format-check

# Static analysis
cmake --build build --target lint

# Full validation (format + lint + test)
cmake --build build --target validate

# Sanitizers (debug builds)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
```

---

## Code Style

### Imports & Headers
- System headers first, then third-party, then project headers (alphabetical within groups)
- Use `#include "..."` for project headers, `<...>` for system/third-party
- One blank line between include groups
- Use forward declarations in headers when possible; include in .cpp

### Formatting
- Base style: LLVM, 4-space indent, 120 column limit, pointer alignment Left
- Braces on same line for functions, new line for namespaces/classes
- No trailing whitespace; end files with newline

### Types & Naming
- `PascalCase` for classes, structs, enums, namespaces
- `camelCase` for functions, methods, variables, parameters
- `m_` prefix for private member variables (e.g., `m_filePath`)
- `k_` prefix for constants (e.g., `kMaxFileSize`)
- Prefer `std::string_view` for read-only string parameters
- Use `std::optional<T>` for nullable returns; avoid raw pointers for ownership
- Use `std::unique_ptr` / `std::shared_ptr` for heap ownership; prefer stack allocation

### Error Handling
- Never silently swallow exceptions or error codes
- Use `std::expected<T, E>` for recoverable failures
- Log at boundary of each major operation (open, save, delete, encoding)
- Surface actionable error messages to user via native dialogs
- Never discard user data on failure; keep editor contents intact

### Logging
- Use **spdlog**; levels: trace, debug, info, warn, error, critical
- Log file paths (not contents); never log sensitive data
- Include component context in messages (e.g., `FileService::LoadFile failed: ...`)

---

## Architecture Rules

- **UI** (`src/ui/`) — presentation only; no disk IO
- **Core** (`src/core/`) — Document, Encoding, FileService, MediaService
- **AI** (`src/ai/`) — AI provider abstraction, media extraction, generation services
- **App** (`src/app/`) — bootstrap, lifecycle
- **Platform** (`src/platform/`) — OS-specific paths/config
- **Python** (`src/python/`) — pybind11 bindings for IDE automation
- UTF-8 is the internal canonical text representation
- Safe save: write to temp file → flush → atomic replace
- RAII throughout; no raw owning pointers
- No Qt or RmlUi; wxWidgets only for UI
- Use `Platform::GetProjectRoot()` for asset paths — never hardcode `/proc/self/exe`
- Register `wxPNGHandler` once in `Application::OnInit()`
- Per-tab `Document` tracking for dirty state, encoding, and file paths
- Every media viewer is a `wxPanel` — dockable, tabbable, consistent behavior
- `MediaService` is the single source of truth for type detection, metadata, AI extraction
- All libraries must be commercially viable at zero cost

---

## Testing

- Framework: **Catch2 v3**, run via **CTest**
- Tests in `tests/`; no UI dependencies; use temp directories
- Coverage targets: Encoding ≥ 90%, FileService ≥ 85%, Document ≥ 85%
- Tests must be deterministic and order-independent
- Clean up temp directories in each test

---

## Prohibited Patterns

- Mixing UI and file parsing logic
- Hardcoded platform-specific paths throughout codebase
- Unnecessary singletons or global mutable state
- Silent failure swallowing
- Background complexity beyond editor needs
- Repeated `wxImage::AddHandler()` calls — register once at startup
- Unused dead code in the repository
- Libraries with restrictive or commercial-use licenses

---

## Logging System

### Framework
Use **spdlog** with rotating file sink (5MB max, 3 files).

### Log levels
- **trace:** internal state transitions
- **debug:** developer diagnostics (file paths, encoding decisions)
- **info:** user-facing lifecycle events (app start, file opened, file saved)
- **warn:** recoverable issues (fallback encoding, non-critical IO errors)
- **error:** failures that block an operation (save failed, file not found)
- **critical:** unrecoverable errors

### Configuration
- Initialize during application startup
- Synchronous logger for deterministic ordering
- Output to **stdout** and **rotating file sink**
- Debug builds: log to `<project_root>/logs/notepad.log`
- Release builds: log to user app data directory
- Default level: `info` for release, `debug` for debug
- wxWidgets log messages routed through spdlog via custom `SpdlogTarget`
- wxAssertions captured via `wxSetAssertHandler` (debug only)
- Filter out focus-related wxWidgets log noise

---

## Code Validation

### Static analysis
- **clang-tidy**: `cppcoreguidelines-*`, `modernize-*`, `readability-*`, `bugprone-*`, `performance-*`
- `.clang-tidy` at project root

### Formatting
- **clang-format**: LLVM base, 4-space indent, 120 column limit
- `.clang-format` at project root

### Compiler warnings
- GCC/Clang: `-Wall -Wextra -Wpedantic`
- MSVC: `/W4`

### Sanitizers (debug builds)
- AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), LeakSanitizer (LSan)
- Enable via `-DENABLE_SANITIZERS=ON`

---

## Current Project State

### Source Files (27 files)
| File | Lines | Purpose |
|------|-------|---------|
| `src/main.cpp` | 11 | Entry point |
| `src/app/Application.hpp` | 21 | wxApp declaration |
| `src/app/Application.cpp` | 143 | Bootstrap, spdlog, wxLog routing, assert handler, PNG/Magick init |
| `src/core/Encoding.hpp` | 70 | BOM detection, UTF-8/16 encode/decode API |
| `src/core/Encoding.cpp` | 416 | Full encoding with surrogate pairs, UTF-8 validation |
| `src/core/Document.hpp` | 48 | Document model, dirty tracking |
| `src/core/Document.cpp` | 74 | Document implementation |
| `src/core/FileService.hpp` | 42 | Safe save, load, delete API |
| `src/core/FileService.cpp` | 162 | Atomic replace, returns DecodeResult with metadata |
| `src/core/MediaService.hpp` | 52 | Media type detection, metadata, AI extraction stubs |
| `src/core/MediaService.cpp` | 141 | Type detection, ImageMagick metadata, pixel data extraction |
| `src/ui/Theme.hpp` | 35 | Dark colour palette |
| `src/ui/Theme.cpp` | 35 | Dark colours + Windows remap |
| `src/ui/EditorPanel.hpp` | 24 | wxTextCtrl subclass |
| `src/ui/EditorPanel.cpp` | 42 | Monospace font, dark colours |
| `src/ui/ImageViewer.hpp` | 45 | Image viewer with zoom, pan, fit |
| `src/ui/ImageViewer.cpp` | 196 | ImageMagick loading, mouse wheel zoom, drag pan, context menu |
| `src/ui/MainFrame.hpp` | 118 | Main window: AUI, IconBar (4 icons), Explorer, PromptBar, ImageViewer |
| `src/ui/MainFrame.cpp` | 804 | Full docking system, media routing, sidebar modes, per-tab Document tracking |
| `src/platform/PlatformPaths.hpp` | 21 | Cross-platform path helpers |
| `src/platform/PlatformPaths.cpp` | 62 | XDG on Linux, APPDATA on Windows |
| `tests/EncodingTests.cpp` | 166 | 16 encoding tests |
| `tests/FileServiceTests.cpp` | 179 | 10 file service tests |
| `tests/DocumentTests.cpp` | 100 | 10 document tests |
| `tests/MediaServiceTests.cpp` | 108 | 10 media service tests |

### Assets
- `assets/canvas.png` — Background image
- `assets/icons/folder_icon_128x128.png` — Sidebar folder icon
- `assets/icons/images_icon.png` — Sidebar images icon
- `assets/icons/video_icon.png` — Sidebar video icon
- `assets/icons/3dmodels_icon.png` — Sidebar 3D models icon
- `assets/icons/send_icon.png` — Send button icon (80×32)
- `assets/icons/clear_icon.png` — Clear button icon (80×32)
- `assets/icons/app_icon.png` — Application icon
- `assets/skills/wxwidgets-reference.md` — wxWidgets reference guide

---

## Delivery Expectations

A valid deliverable should include:
- Complete C++ source
- Complete CMake configuration
- Buildable project structure
- Dark theme implementation
- CRUD text-file support
- Unicode-safe file handling
- Clean startup/shutdown behavior
- Logging system with rotating file output
- Automated tests with ≥ 85% coverage on core modules
- clang-tidy and clang-format configuration
- Sanitizer support in debug builds
- Validation CMake targets
- README with build instructions for Linux and Windows x64
- All libraries commercially viable at zero cost

---

## Milestone Roadmap

Development follows the 8-milestone roadmap in `milestones.md`:

1. ✅ **Core IDE Shell** — wxAui docking, text editor, logging, tests
2. ⏳ **Image Processing** — ImageViewer, MediaService, IconBar with 4 modes, file explorer with file selection
3. ⏳ **Video Player** — libmpv playback, scrub bar, frame extraction
4. ⏳ **Audio Player** — libmpv audio, waveform visualization
5. ⏳ **3D Model Viewer** — Assimp + OpenGL, orbit camera
6. ⏳ **AI Integration Layer** — AI provider abstraction, media extraction
7. ⏳ **AI Content Generation** — Text-to-image/video/model generation
8. ⏳ **Unified Workspace** — Split workspace, asset browser, full IDE

---

## Codex Response Style
When performing coding tasks for this project, Codex should structure responses using this schema unless the user requests otherwise:

### ANALYSIS
What the task requires and key constraints.

### PLAN
What will be changed.

### CHANGES
Files added or modified.

### CODE
Relevant code or patch.

### VALIDATION
How the result was checked.

### RISKS
Any known limitations or follow-up concerns.

---

## Final Directive
If there is a conflict between simplicity and unnecessary framework complexity, choose simplicity.

The target product is **CLIADE AI Content Creator** — a reliable, commercial-friendly, cross-platform AIO IDE in **C++23**, designed for **Linux x64** and **Windows x64**, developed comfortably in **CLion**, with a workflow compatible with **OpenCode + Qwen3.6**. It combines code editing, media viewing (AVIM Canvas), and AI-powered content generation in one unified workspace.

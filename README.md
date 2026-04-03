# CLIADE NotePad

A cross-platform C++23 desktop text editor built with wxWidgets 3.2+.

**AJC-Software Ltd © 2026**

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
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run
./build/CLIADE
```

### Release Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### With Sanitizers (Debug)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build build
```

## Testing

```bash
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

# Static analysis
cmake --build build --target lint

# Full validation (format + lint + test)
cmake --build build --target validate
```

## Project Structure

```
CLIADE/
├── CMakeLists.txt                    # C++23, wxWidgets/spdlog/Catch2, sanitizers, format/lint/test/validate targets
├── AGENTS.md                         # Agent instructions and code standards
├── README.md                         # This file
├── .gitignore                        # Git ignore rules
├── .clang-format                     # LLVM base, 4-space indent, 120 column limit
├── .clang-tidy                       # cppcoreguidelines/modernize/readability/bugprone/performance checks
├── resources.rc                      # Windows application icon resource
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
│   └── platform/
│       ├── PlatformPaths.hpp         # Cross-platform app data/log/project root path helpers
│       └── PlatformPaths.cpp         # XDG on Linux, APPDATA on Windows, /proc/self/exe resolution
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
    │   └── app_icon.png              # Application window/taskbar icon
    └── skills/
        └── wxwidgets-reference.md    # wxWidgets reference extracted from PDF
```

## Architecture

- **UI** (`src/ui/`) — Presentation only; no disk IO
- **Core** (`src/core/`) — Document, Encoding, FileService
- **App** (`src/app/`) — Bootstrap, lifecycle, logging init
- **Platform** (`src/platform/`) — OS-specific paths/config

## License

AJC-Software Ltd © 2026

# UI v2 Navigation — Primary Modes and File Access

## Primary Modes (Frozen Order)
The primary workspace modes are fixed in the following order:

1. Notepad
2. Images
3. Video
4. Models
5. AI *(optional; feature-flagged or hidden when unavailable)*
6. Settings

### Rules
- This order is canonical for sidebar/toolstrip rendering, keyboard mode cycling, and persisted UI state.
- New modes must not be inserted before these without a formal navigation revision.
- If AI mode is unavailable, it is omitted without reordering the remaining modes.

## Explorer Mode in Default User Flow
Explorer mode is removed from the default user flow.

### Clarification
- Explorer is **not** a primary mode.
- Users should land in and navigate through the frozen primary mode list above.
- Any file browsing UI that remains is a supporting panel/action inside a mode, not a top-level mode destination.

## File Access Entry Points (Exact Behavior)

### 1) File > Open
- Opens the platform native file picker.
- Allows selecting one or more files (subject to current implementation limits).
- On confirm, each selected file is opened in the workspace using existing media/type routing.
- The successfully opened file paths are added to Open Recent (most-recent-first, de-duplicated).

### 2) File > Open Recent
- Shows a bounded MRU list of previously opened files.
- Selecting an item attempts to open that exact path.
- If the file no longer exists or cannot be accessed, show a user-facing error and remove (or mark invalid) that MRU entry.
- Re-opening a valid recent file moves it to the top of MRU.

### 3) Quick Open (Shortcut)
- Shortcut invokes a lightweight quick-open surface (command palette / quick picker).
- Default shortcut: `Ctrl+P` on Windows/Linux (`Cmd+P` on macOS when applicable).
- Supports fuzzy matching over project/workspace-visible files and recent files.
- Choosing an entry opens the file via the same routing pipeline as File > Open.

### 4) Drag-and-Drop
- Dragging file(s) from the OS into the main window opens them directly.
- Drop handling uses the same routing/open pipeline as File > Open.
- Successfully opened dropped files are inserted into Open Recent.
- Unsupported items (directories, unknown/blocked types per current policy) produce clear non-blocking feedback.

## Unification Requirement
All four entry points must converge on one shared "open file" workflow to guarantee:
- consistent mode routing behavior,
- consistent error handling/messages,
- consistent Open Recent updates,
- reduced divergence and regression risk.

## Out of Scope for This Note
- Detailed visual layout/styling of mode controls.
- Final MRU maximum size value.
- Advanced Quick Open ranking heuristics.

## Icon Asset Mapping Update
- Notepad mode icon asset path: `assets/icons/notepad_icon.png`.

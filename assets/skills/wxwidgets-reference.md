# wxWidgets Reference Guide
## Extracted from "Cross-Platform GUI Programming with wxWidgets" by Julian Smart & Kevin Hock

---

## Application Architecture

### wxApp (Application Class)
- Every wxWidgets app defines a class deriving from `wxApp`
- Only one instance exists, representing the running application
- Must define `virtual bool OnInit()` — called when wxWidgets is ready
- Return `true` to start event loop, `false` to terminate
- Top-level windows must be shown explicitly with `Show(true)`
- `wxTheApp` global variable points to the application instance
- `DECLARE_APP(MyApp)` / `wxGetApp()` for typed access
- `IMPLEMENT_APP(MyApp)` tells wxWidgets what class to instantiate (also checks build config match)
- `OnExit()` called only if `OnInit()` returns `true`

### Program Flow
1. wxWidgets main/WinMain runs (supplied by library)
2. wxWidgets creates instance of your `wxApp` subclass
3. `OnInit()` is called — create frames, setup data
4. `OnInit()` returns `true` → event loop starts
5. Application terminates when last top-level window closes

---

## Window Hierarchy

### Top-Level Windows
- **wxFrame** — main application window with title bar, menu bar, status bar
- **wxDialog** — modal or modeless dialogs for user interaction
- **wxMiniFrame** — smaller caption frame (tool palettes)
- **wxPopupWindow** — brief popups (menus, tooltips)

### Container Windows
- **wxPanel** — container for controls on non-dialog windows (use for notebook pages)
  - Default style: `wxCAP_TRAVERSAL | wxNO_BORDER`
  - Handles Tab key navigation automatically
  - Use `InitDialog` to send `wxInitDialogEvent` for validator data transfer
- **wxNotebook** — tabbed interface with multiple pages
  - Pages are typically `wxPanel` or derived classes
  - Use `AddPage()` / `InsertPage()` — do NOT explicitly destroy managed pages
  - `wxNB_NOPAGETHEME` suppresses Windows XP gradient (improves performance)
  - Events: `EVT_NOTEBOOK_PAGE_CHANGED`, `EVT_NOTEBOOK_PAGE_CHANGING` (can veto)
  - Alternatives: `wxListbook`, `wxChoicebook` (same API, different UI)
- **wxScrolledWindow** — automatic scrollbar management
  - `SetScrollbars(pixelsPerUnitX, pixelsPerUnitY, noUnitsX, noUnitsY)`
  - Call `DoPrepareDC(dc)` before drawing to set device origin
  - Or override `OnDraw(wxDC& dc)` — DC is prepared automatically
- **wxSplitterWindow** — resizable split panes

### Non-Static Controls
- **wxButton** — standard push button
- **wxBitmapButton** — button with bitmap instead of text
- **wxTextCtrl** — text input (single or multi-line)
  - Styles: `wxTE_MULTILINE`, `wxTE_PROCESS_ENTER`, `wxTE_READONLY`
  - Events: `EVT_TEXT`, `EVT_TEXT_ENTER`, `EVT_CHAR`
- **wxCheckBox**, **wxRadioButton**, **wxRadioBox**
- **wxChoice**, **wxComboBox**, **wxListBox**
- **wxSlider**, **wxSpinButton**, **wxSpinCtrl**
- **wxToggleButton**

### Static Controls
- **wxStaticText**, **wxStaticBitmap**, **wxStaticLine**, **wxStaticBox**
- **wxGauge** — progress indicator

---

## Event Handling

### Event Tables (Static)
```cpp
// In header:
DECLARE_EVENT_TABLE()

// In implementation:
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
    EVT_SIZE(MyFrame::OnSize)
    EVT_BUTTON(wxID_OK, MyFrame::OnButtonOK)
END_EVENT_TABLE()
```

### Dynamic Event Handlers
```cpp
frame->Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler(MyFrame::OnQuit));
frame->Disconnect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
                  wxCommandEventHandler(MyFrame::OnQuit));
```

### Event Propagation
- **Command events** (wxCommandEvent-based) propagate up window hierarchy
- **System events** do NOT propagate: wxActivateEvent, wxCloseEvent, wxEraseEvent,
  wxFocusEvent, wxKeyEvent, wxIdleEvent, wxInitDialogEvent, wxJoystickEvent,
  wxMenuEvent, wxMouseEvent, wxMoveEvent, wxPaintEvent, wxQueryLayoutInfoEvent,
  wxSizeEvent, wxScrollWinEvent, wxSysColourChangedEvent

### Skipping Events
- Call `event.Skip()` to continue event search (like calling base class virtual)
- Don't call `Skip()` to consume the event

### Event Handler Functions
- Return type: `void`
- NOT virtual
- Single event object argument
- Handler signature varies by event type

---

## Window Identifiers

### Standard Identifiers (use these!)
- `wxID_OPEN`, `wxID_CLOSE`, `wxID_NEW`, `wxID_SAVE`, `wxID_SAVEAS`
- `wxID_EXIT`, `wxID_UNDO`, `wxID_REDO`
- `wxID_CUT`, `wxID_COPY`, `wxID_PASTE`, `wxID_SELECTALL`
- `wxID_OK`, `wxID_CANCEL`, `wxID_APPLY`, `wxID_YES`, `wxID_NO`
- `wxID_HELP`, `wxID_ABOUT`, `wxID_PRINT`, `wxID_FIND`
- `wxID_ANY` — auto-generate identifier (always negative, won't conflict)
- `wxID_HIGHEST` (5999) — safe to define custom IDs above this
- Some systems use standard IDs for default graphics/behavior (GTK+ OK/Cancel buttons)
- `wxTextCtrl` knows how to handle `wxID_COPY`, `wxID_PASTE`, `wxID_UNDO`

---

## Frame (wxFrame)

### Constructor
```cpp
wxFrame(wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE,
        const wxString& name = wxT("frame"));
```

### Styles
- `wxDEFAULT_FRAME_STYLE` = `wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX`
- `wxFRAME_FLOAT_ON_PARENT` — always on top of parent (must have non-NULL parent)
- `wxFRAME_NO_TASKBAR` — doesn't appear in taskbar
- `wxFRAME_SHAPED` — allows `SetShape()` for non-rectangular frames
- `wxSTAY_ON_TOP` — Windows only

### Key Functions
- `CreateStatusBar(nFields, style)` — creates status fields
- `SetStatusWidths(n, widths[])` — customize field widths (-1 = remaining space)
- `SetStatusText(text, field)` — set text for a field
- `CreateToolBar()` — creates toolbar under menu bar
- `SetMenuBar(wxMenuBar*)` — attach menu bar (replaces old one, old is deleted)
- `SetTitle()` / `GetTitle()` — title bar text
- `SetIcon(wxIcon)` — taskbar/minimized icon
- `SetIcons(wxIconBundle)` — multiple resolutions/depths
- `ShowFullScreen()` — hide decorations, maximize client area
- **Destroy windows with `Destroy()` or `Close()`, NOT `delete`**
  - `Close()` generates `wxEVT_CLOSE_WINDOW`, default handler calls `Destroy()`
  - Destruction delayed until idle time

---

## Dialog (wxDialog)

### Modal vs Modeless
- **Modal**: `ShowModal()` — blocks until dismissed, returns identifier
- **Modeless**: `Show()` — like a frame, doesn't block
- Modal dialogs can be created on stack (rare for wxWindow-derived objects)

### Key Functions
- `EndModal(int retCode)` — exits modal loop, retCode returned by `ShowModal()`
- Default `wxEVT_CLOSE` handler simulates `wxID_CANCEL`
- `wxDIALOG_NO_PARENT` — creates orphan dialog (not recommended for modal)
- `wxDIALOG_EX_CONTEXTHELP` — Windows query button on caption

---

## Menus and Toolbars

### wxMenuBar
```cpp
wxMenu *fileMenu = new wxMenu;
fileMenu->Append(wxID_EXIT, wxT("E&xit\tAlt-X"), wxT("Quit this program"));
wxMenuBar *menuBar = new wxMenuBar();
menuBar->Append(fileMenu, wxT("&File"));
frame->SetMenuBar(menuBar);
```
- Mnemonic: `&` before letter (Alt+letter when menu open)
- Accelerator: `\t` followed by key combo (e.g., `\tCtrl+S`)
- Help string shown on status bar when hovering

### wxToolBar
- Tool bitmap colors under Windows may need special handling
- Create with `CreateToolBar()` or construct and call `SetToolBar()`

---

## Window Layout (Sizers)

### Common Sizers
- **wxBoxSizer** — horizontal or vertical layout
- **wxStaticBoxSizer** — box with label around controls
- **wxGridSizer** — uniform grid
- **wxFlexGridSizer** — flexible rows/columns
- **wxGridBagSizer** — grid with spanning cells

### Sizer Programming
```cpp
wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
sizer->Add(control, proportion, flags, border);
window->SetSizer(sizer);
```
- **proportion**: 0 = fixed size, >0 = share extra space
- **flags**: `wxEXPAND`, `wxALL`, `wxLEFT`, `wxRIGHT`, `wxTOP`, `wxBOTTOM`
- **border**: pixels around control

### Layout Tips
- Use dialog units for platform-adaptive sizing
- `wxDefaultSize` / `wxDefaultPosition` — let control choose
- `DoGetBestSize()` — control's reasonable default based on content/font
- Dynamic layouts: `Layout()` to recalculate after changes

---

## Drawing and Device Contexts

### DC Types
- **wxClientDC** — draw on window outside paint event
- **wxPaintDC** — MUST use in `wxEVT_PAINT` handler
- **wxMemoryDC** — draw on bitmap
- **wxScreenDC** — draw on entire screen
- **wxPrinterDC** / **wxPostScriptDC** — printing

### Painting
- Window receives `wxEVT_ERASE_BACKGROUND` then `wxEVT_PAINT`
- Use `wxPaintDC` in paint handler — never `wxClientDC`
- Get update region with `GetUpdateRegion()` for optimization
- `SetBackgroundStyle(wxBG_STYLE_CUSTOM)` — control background erasing

### Drawing Tools
- **wxPen** — line color/style/width
- **wxBrush** — fill color/style
- **wxFont** — text font
- **wxColour** — RGB color

---

## Images and Icons

### wxImage
- Platform-independent image manipulation
- Load: `wxImage(filename, type)`
- Manipulate: `Rescale()`, `ConvertToGreyscale()`, `SetRGB()`
- Direct pixel access for custom manipulation

### wxBitmap
- Platform-dependent bitmap for drawing
- Create from wxImage: `wxBitmap(image)`
- Draw with `dc.DrawBitmap(bitmap, x, y, useMask)`

### wxIcon
- Small bitmap for window icons
- XPM format works on all platforms (valid C++ syntax, can `#include`)
- `SetIcon(wxIcon(xpm_data))` — frame/taskbar icon
- `SetIcons(wxIconBundle)` — multiple resolutions

---

## Input Handling

### Mouse Events
- Button events: `EVT_LEFT_DOWN`, `EVT_LEFT_UP`, `EVT_LEFT_DCLICK`
- Motion events: `EVT_MOTION` (need `CaptureMouse()` for tracking outside window)
- Wheel events: `EVT_MOUSEWHEEL`

### Keyboard Events
- `EVT_CHAR` — character events (after translation)
- `EVT_KEY_DOWN` / `EVT_KEY_UP` — raw key events
- Call `event.Skip()` to allow default processing
- `wxIsalpha()`, `wxIsdigit()` for key code checking

### Accelerators
```cpp
wxAcceleratorEntry entries[] = {
    wxAcceleratorEntry(wxACCEL_CTRL, (int)'S', wxID_SAVE),
    wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'S', wxID_SAVEAS),
};
frame->SetAcceleratorTable(wxAcceleratorTable(2, entries));
```

---

## Memory Management

### Window Ownership
- Windows created with `new` are owned by their parent
- Parent deletes children automatically
- Top-level windows (frames, dialogs) are NOT deleted by parent
- **Use `Destroy()` or `Close()`, NOT `delete`** for windows
- Destruction delayed until idle time to process pending events

### Event Handlers
- `PushEventHandler()` / `PopEventHandler()` — stack of handlers
- Top handler gets events first
- `PopEventHandler(true)` deletes the popped handler

---

## Common Patterns

### Frame Constructor Pattern
```cpp
MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title)
{
    SetIcon(wxIcon(icon_xpm));

    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT, "E&xit");
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About");

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar(2);
    SetStatusText("Ready");
}
```

### Dialog Pattern
```cpp
void ShowDialog() {
    MyDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        // User confirmed
    }
}

// In dialog:
void MyDialog::OnOK(wxCommandEvent& event) {
    EndModal(wxID_OK);
}
```

### Close Handler Pattern
```cpp
void MyFrame::OnClose(wxCloseEvent& event) {
    if (HasUnsavedChanges()) {
        int result = wxMessageBox("Save changes?", "Confirm",
                                  wxYES_NO | wxCANCEL);
        if (result == wxCANCEL) {
            event.Veto();
            return;
        }
        if (result == wxYES && !SaveChanges()) {
            event.Veto();
            return;
        }
    }
    event.Skip(); // Allow default destruction
}
```

---

## Platform Considerations

### GTK+ (Linux)
- Window shape must be set after `EVT_WINDOW_CREATE`
- MDI emulated with tabbed windows
- Standard IDs provide native button graphics

### Windows (MSW)
- Tool bitmap colors may need special handling
- `wxMiniFrame` has smaller caption
- MDI uses native MDI client window

### Mac OS X
- `wxID_ABOUT`, `wxID_PREFERENCES`, `wxID_EXIT` moved to application menu
- Tabs don't scroll on wxNotebook (limited by window width)
- `wxDIALOG_EX_METAL` for metallic appearance

---

## Best Practices

1. **Use standard identifiers** (`wxID_*`) wherever possible
2. **Call `event.Skip()`** when you want default processing to continue
3. **Use sizers** for layout — never hardcode positions
4. **Destroy windows with `Destroy()` or `Close()`**, never `delete`
5. **Use `wxPaintDC`** only in paint event handlers
6. **Call `DoPrepareDC(dc)`** before drawing on scrolled windows
7. **Show top-level windows explicitly** with `Show(true)`
8. **Use `wxDefaultSize`/`wxDefaultPosition`** to let controls choose optimal size
9. **Handle `wxEVT_CLOSE`** for confirmation dialogs before window destruction
10. **Use `wxSystemOptions::SetOption()`** for platform-specific behavior tweaks

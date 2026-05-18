# Package layout

Contents of `repos-folder/giu/` (flat package — everything is `package giu`):

```
giu/
├── go.mod
├── go.sum
├── README.md / LICENSE / migration.txt
│
├── doc.go                     Package doc comment
│
├── MasterWindow.go            OS window + glfw backend wrapper
├── Backend.go                 cimgui-go backend abstraction
├── Context.go                 *GIUContext singleton, GenAutoID, SetState[T] / GetState[T]
│
├── Layout.go                  Layout = []Widget — the rendering loop entry
├── Widgets.go                 Row, Column, Combo, Menu, TabBar, Tooltip, ProgressBar, ColorEdit, ContextMenu
├── Window.go                  SingleWindow, SingleWindowWithMenuBar, Window
├── TextWidgets.go             Label, InputText, InputInt/Float, BulletText
├── ClickableWidgets.go        Button, Checkbox, RadioButton, Selectable, TreeNode, Link, ImageButton
├── SliderWidgets.go           SliderInt/Float, VSliderInt, DragInt/Float
├── ImageWidgets.go            Image, ImageWithRgba, ImageWithFile, ImageWithURL
├── TableWidgets.go            Table, TableRow, TableColumn, TreeTable
├── ListClipper.go             Virtualized list helpers
├── ExtraWidgets.go            DatePicker, ListBox, Splitter, RangeBuilder, MapRangeBuilder, Custom, Condition
├── Popups.go                  Popup, PopupModal, OpenPopup, CloseCurrentPopup
├── Msgbox.go                  PrepareMsgbox, Msgbox
├── Markdown.go                immarkdown wrapper
├── CodeEditor.go              ImGuiColorTextEdit wrapper
├── MemoryEditor.go            imgui_memory_editor wrapper
├── Gizmo.go                   imguizmo wrapper (Grid, Cube, Manipulate, ViewManipulate, ViewMatrix, ProjectionMatrix)
├── Plot.go                    implot wrapper (Bar, Line, Scatter, PieChart, etc.)
├── ProgressIndicator.go       Spinner
├── Canvas.go                  Low-level draw list (Lines, Paths, Text, Images)
├── SplitLayout.go             SplitLayout(direction, sashPos, w1, w2)
├── StackWidget.go             Stack(visibleIdx, layouts...) — card stack
│
├── Style.go                   PushColor*, PushStyle*, PopStyle*, PushFont, PopFont, GetWindowPadding, ...
├── StyleSetter.go             *StyleSetter — fluent style stack applied via .To(widgets...)
├── StyleIDs.go                StyleColorID, StyleVarID, StylePlotColorID, StylePlotVarID consts
├── stylecolorid_enumer.go     Generated enumer Strings
├── stylevarid_enumer.go       Generated enumer Strings
├── styleplotcolorid_enumer.go
├── styleplotvarid_enumer.go
├── Themes.go                  DefaultTheme, LightTheme
├── CSS.go                     CSS parsing / tag system
│
├── FontAtlasProsessor.go      Incremental font atlas rebuild
├── Translator.go              i18n hook (Translator interface, EmptyTranslator, NewBasicTranslator)
├── SurfaceLoaders.go          Async image fetch/decode (used by ImageWithURL)
├── ReflectiveBoundTexture.go  Texture state bridge (rebuilds when image changes)
├── StatefulReflectiveBoundTexture.go
├── Texture.go                 *Texture, ToTexture, NewTextureFromRgba, EnqueueNewTextureFromRgba
├── TextureFilters.go          Filtering options
│
├── EventHandler.go            giu.Event() — hover/active/click hook on the last submitted widget
├── InputHandler.go            Keyboard shortcut handling
├── InputHandler_test.go
├── Keycode.go                 Key constants
├── Direction.go               Direction enum (arrows etc.)
├── Alignment.go               Alignment enum
├── Flags.go                   ImGui flag re-exports (WindowFlags, TreeNodeFlags, TableFlags, ...)
├── surfacestate_string.go     Generated Stringer
│
├── mainthread_all.go          (build tags decide which file compiles)
├── mainthread_mac.go          macOS main-thread routing (via mainthread)
├── mainthread_windows.go      Windows main-thread routing
├── build_windows.go           Build flags for Windows
│
├── Utils.go                   Image conversion, geometry, frame helpers (Update, GetMousePos, CalcTextSize, SetNextWindowSize, ...)
├── Utils_test.go
├── Context_test.go
├── Layout_test.go
├── StyleSetter_test.go
│
├── cmd/                       Companion CLI(s)
├── docs/                      Supplementary docs / wiki sources
├── examples/                  Example apps (helloworld, gizmo, codeeditor, plot, ...)
└── screenshots/               README screenshots
```

## Module identity

```
module github.com/AllenDang/giu
go 1.25.0
```

## Major dependencies (from go.mod)

| Module | Purpose |
|---|---|
| `github.com/AllenDang/cimgui-go` | cgo Dear ImGui binding (imgui, implot, immarkdown, imnodes, imguizmo, backend/glfwbackend) |
| `github.com/AllenDang/go-findfont` | OS font discovery |
| `github.com/faiface/mainthread` | Main-thread routing (legacy) |
| `golang.design/x/mainthread` | Main-thread routing (current) |
| `github.com/gucio321/glm-go` | Float math (matrices for gizmos) |
| `github.com/mazznoer/csscolorparser` | CSS color parsing |
| `github.com/napsy/go-css` | CSS rule parsing |
| `github.com/pkg/browser` | Open URLs |
| `github.com/sahilm/fuzzy` | Fuzzy search |
| `golang.design/x/hotkey` | Global hotkeys |
| `golang.org/x/image` | Image utilities |
| `gopkg.in/eapache/queue.v1` | Texture loading queue |
| `github.com/stretchr/testify` | Test asserts |

## Build prerequisites

- A C/C++ toolchain (MinGW-w64 on Windows, Xcode CLI on macOS, gcc on Linux) — cgo links the bundled ImGui C++.
- GLFW v3.3 (linked via cimgui-go).
- OpenGL 3.2 driver.

## Cross-compilation

Cross-compiling from one OS to another usually requires the target OS's libraries available to the linker. The most reliable path is to build natively per platform; CI matrices typically run one job per (windows, macos, linux) tuple.

## Platforms

Windows 10/11 x64, macOS ≥10.15 (incl. Big Sur and later), Linux (X11 and Wayland via GLFW). Raspberry Pi 3B is reported working. iOS / Android / WASM are not supported by cimgui-go's build matrix.

# Samples

`samples/` contains ~95 example programs. Each subdirectory builds a small
self-contained app. A useful subset to read first:

| Sample | Demonstrates |
|--------|--------------|
| `minimal` | The smallest possible wxWidgets app: a frame, a menu, About/Quit |
| `widgets` | A tour of every standard control with live properties |
| `dialogs` | Stock and custom dialogs (file, font, color, message, etc.) |
| `layout` | Sizer-based layout |
| `aui` | Advanced UI (docking panes, perspectives) |
| `xrc` | UI loaded from XML resource files |
| `docview` | Document/View framework |
| `dnd` | Drag and drop |
| `drawing` | `wxDC` 2D drawing primitives |
| `opengl` | OpenGL inside a wxWindow via `wxGLCanvas` |
| `webview` | Embedded browser (`wxWebView`) |
| `webrequest` | HTTP requests via the native stack |
| `richtext`, `stc` | Rich text control and Scintilla text control |
| `propgrid` | Property grid editor |
| `printing` | Print framework |
| `mediaplayer` | `wxMediaCtrl` audio/video |
| `thread` | `wxThread` patterns |
| `sockets` | `wxSocket` client/server |
| `console` | Non-GUI wx use (CLI-style apps) |
| `internat` | i18n / gettext catalogs |
| `taskbar`, `taskbarbutton` | Tray icons / Windows taskbar buttons |
| `secretstore` | Native credential store |
| `power` | Power / battery events |

Solution / project files for samples: `samples_vc14.sln` through
`samples_vc17.sln` plus `samples_vc18.slnx` (MSVS 2026).

The `samples/minimal/minimal.cpp` source itself is reproduced in
[hello-world.md](hello-world.md).

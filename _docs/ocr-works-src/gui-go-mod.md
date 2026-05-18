# src/gui/go.mod

Module `orcgui`, Go 1.26.2.

Direct deps:

- `fyne.io/fyne/v2 v2.7.4` — GUI toolkit (`app`, `canvas`, `container`, `dialog`, `widget`, `theme`).
- `github.com/sqweek/dialog` — native OS file-open dialog (imported as `nfd`); used for the **Open PDF** button.
- `github.com/shirou/gopsutil/v4` — CPU + RAM sampling for the metrics column ([metrics.go](../../src/gui/metrics.go)).

Everything else in the file is transitive: Fyne pulls in OpenGL/GLFW bindings, image codecs, text shaping, systray, dbus/portal for Linux desktop integration, and i18n. gopsutil pulls in `tklauser/go-sysconf`, `go-ole`, `yusufpapurcu/wmi`, etc. for the various platform backends.

# Installation

Fyne is a Go module. Add it to your project:

```
go get fyne.io/fyne/v2@latest
go mod tidy
```

Then `import "fyne.io/fyne/v2/app"` and friends.

## Prerequisites

- Go 1.19+ (declared in `go.mod`; the README mentions 1.17 but the module pins 1.19).
- A C compiler — fyne uses cgo to link against the platform's OpenGL stack via `github.com/go-gl/glfw/v3.3/glfw`.
  - **Windows**: a working `gcc` (MinGW-w64 via msys2 or the bundled toolchain in `tdm-gcc`).
  - **macOS**: Xcode command line tools (`xcode-select --install`).
  - **Linux**: `gcc` plus the X / Wayland dev headers (`libgl1-mesa-dev`, `libxcursor-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxi-dev`, `xorg-dev` — exact names depend on distro).
- For mobile builds: the Android SDK + NDK (Android), or Xcode (iOS / iOS simulator).

> The first compile on Windows can take up to 10 minutes (cgo compiling GLFW). Subsequent builds are fast.

## The `fyne` CLI

Installed separately. It packages apps for distribution and installs them into the OS application directory.

```
go install fyne.io/tools/cmd/fyne@latest
```

Common subcommands:

| Command | Purpose |
|---|---|
| `fyne package` | Build a distributable bundle for the current OS, or cross-target via `-os` (`windows`, `darwin`, `linux`, `android`, `ios`, `iossimulator`, `wasm`, `web`) |
| `fyne install` | Same as package then drop the result in the OS standard install location |
| `fyne release` | Sign + bundle for store submission |
| `fyne serve` | Local web server for WASM builds |
| `fyne get` | Install an app from a Go-importable repo |
| `fyne env` | Print build environment info — useful for diagnosing toolchain issues |
| `fyne build` | Just compile, no packaging |
| `fyne version` | Tool / framework version info |

Note: the upstream `fyne` CLI source lives in [cmd/fyne/](../../repos-folder/fyne/cmd/fyne/) inside this repo, but the recommended way to install it is from the standalone `fyne.io/tools` module.

## Mobile packaging

```
fyne package -os android -appID my.domain.appname
fyne install  -os android
```

`-os ios` produces a `.ipa` for real devices; `-os iossimulator` produces a sim-only build.

Mobile simulation locally without packaging:

```
go run -tags mobile main.go
```

## Optional helpers

| Tool | Purpose |
|---|---|
| `fyne.io/fyne/v2/cmd/fyne_settings` | GUI app for managing Fyne global settings (theme variant, scaling) |
| `fyne.io/fyne/v2/cmd/fyne_demo` | The widget showcase (`go install fyne.io/demo@latest && demo`) |
| `github.com/fyne-io/apps` | Graphical installer for apps from apps.fyne.io |

## Single-binary distribution

Fyne apps are statically linked (modulo the OS-provided OpenGL runtime). No additional libraries are required on the target machine — drop the binary on the user's system and it runs. This is one of the headline trade-offs vs Electron-style toolkits: small binaries, no per-app browser.

## Source build

```
git clone https://github.com/fyne-io/fyne
cd fyne
go test ./...
```

The `cmd/fyne_demo` binary can be run from the repo with `go run ./cmd/fyne_demo`. Top-level tests assume a working OpenGL context; CI uses Xvfb on Linux. Pure-Go non-render code (geometry, layouts, theme) tests fine on any host.

## Build tags

- `mobile` — mobile-simulator driver on desktop (`go run -tags mobile`).
- `ci` — switches `app.New()` to an in-memory app for headless testing.
- `release` — strips debug info, sets `Release=true` in `AppMetadata`.
- `flatpak` — Flatpak-specific portal integration for file dialogs / settings.

Various OS files use `//go:build darwin`, `//go:build windows`, `//go:build android`, `//go:build ios`, `//go:build wasm`, `//go:build js`, `//go:build linux || freebsd || openbsd || netbsd` style constraints — the trailing `_<os>.go` suffix is the most common pattern.

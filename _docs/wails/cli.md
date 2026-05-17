# CLI — `wails3`

The v3 CLI lives in [`v3/cmd/wails3/main.go`](../../repos-folder/wails/v3/cmd/wails3/main.go) and is built on the `clir` library. Install:

```
go install github.com/wailsapp/wails/v3/cmd/wails3@latest
```

## Subcommands

| Command | What it does |
| --- | --- |
| `wails3 init` | Scaffold a new project from a template. |
| `wails3 dev` | Run in dev mode — hot-reload frontend, rebuild Go on file change, open dev tools. |
| `wails3 build` | Compile the Go binary with the frontend bundled in. Flags via `flags.Build` (target OS / arch / debug / ldflags). |
| `wails3 package` | Produce platform-specific installers (`.app`, `.msi`, `.deb`, `.rpm`, `.AppImage`). |
| `wails3 doctor` | System status report — checks compiler, WebView2, GTK, signing tooling. |
| `wails3 doctor-ng` | Same idea, new TUI. |
| `wails3 task` | Wrapper around [Taskfile.dev](https://taskfile.dev) — wails projects ship a `Taskfile.yaml` for build orchestration. |
| `wails3 docs` | Open <https://v3.wails.io> in your browser. |
| `wails3 releasenotes` | Print release notes for the installed version. |

(See `v3/cmd/wails3/main.go` for the full list including subcommands not in this table.)

## A typical project layout (after `wails3 init`)

```
my-app/
├── main.go                  Entry point — application.New + Run
├── go.mod / go.sum          Go module
├── Taskfile.yaml            wails3 task definitions (build, dev, package, …)
├── build/                   Platform-specific build assets (icons, installer config)
│   ├── windows/             AppManifest, MSI WiX config
│   ├── darwin/              Info.plist, entitlements, icon.icns
│   └── linux/               .desktop file, AppImage config
├── frontend/                Whatever frontend you chose
│   ├── package.json
│   ├── index.html
│   └── src/                 React / Svelte / Vue / etc.
└── bindings/                Auto-generated TS/JS from your Go services
```

## Build vs Package

- **`wails3 build`** produces the executable but does not wrap it. Good enough for "double-click to run" on dev machines.
- **`wails3 package`** produces a platform-native installer with proper metadata, code-signing hooks, and auto-updater integration (where supported).

## Taskfile

Each scaffolded project ships a `Taskfile.yaml`. Common tasks:

```
task build
task dev
task package
task lint
task test
```

The CLI's `wails3 task` is a thin wrapper around the standard `task` runner so you don't need it installed globally.

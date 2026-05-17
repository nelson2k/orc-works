# JS API — `@tauri-apps/api`

The npm package shipped from `packages/api/`. It compiles to both CJS and ESM and is the canonical frontend client for talking to the Rust backend. When `app.withGlobalTauri` is true in `tauri.conf.json`, every module below is also reachable as `window.__TAURI__.<module>`.

## Module surface (`src/index.ts`)

```ts
import { app, core, dpi, event, image, menu, mocks, path,
         tray, webview, webviewWindow, window } from '@tauri-apps/api'
```

| Module | File | Purpose |
| --- | --- | --- |
| `core` | `core.ts` | `invoke()`, `Channel`, `addPluginListener`, `PermissionState`, `checkPermissions`, `requestPermissions`, `convertFileSrc`, `isTauri`, `Resource`, `SERIALIZE_TO_IPC_FN`. The IPC primitives. |
| `app` | `app.ts` | App-level info (name, version, identifier). Macros for data-store IDs (macOS/iOS). |
| `event` | `event.ts` | `listen` / `once` / `emit` / `emitTo`. Pub/sub across windows/webviews. |
| `path` | `path.ts` | Path joining and OS-aware locations (`appDataDir`, `documentDir`, `resourceDir`, etc.) — backed by `tauri::path`. |
| `window` | `window.ts` | `Window` class — create, close, move, resize, listen to per-window events. `getCurrentWindow()` for the calling window. |
| `webview` | `webview.ts` | `Webview` class — create child webviews, communicate between them. `getCurrentWebview()`. |
| `webviewWindow` | `webviewWindow.ts` | `WebviewWindow` — convenience class merging a Window and its Webview. |
| `menu` | `menu.ts` | Native menu primitives — `Menu`, `Submenu`, `MenuItem`, `IconMenuItem`, `CheckMenuItem`, `PredefinedMenuItem`. Desktop only. |
| `tray` | `tray.ts` | `TrayIcon` class. Requires the `tray-icon` Cargo feature. |
| `image` | `image.ts` | `Image` and `ImageSize` — image data the menu / tray APIs accept. |
| `dpi` | `dpi.ts` | `LogicalPosition`, `LogicalSize`, `PhysicalPosition`, `PhysicalSize` — scaling-aware geometry. |
| `mocks` | `mocks.ts` | `mockIPC`, `mockWindows`, `clearMocks` for unit testing without a backend. |

## Globally available variant

If `withGlobalTauri = true`, every export above is mirrored under `window.__TAURI__`. The `core` module is `window.__TAURI__.core`, `event` is `window.__TAURI__.event`, and so on. The `SERIALIZE_TO_IPC_FN` symbol is the magic key for custom IPC serialization (see [commands_ipc.md](commands_ipc.md)).

## Mocking for tests

```ts
import { mockIPC } from '@tauri-apps/api/mocks'

mockIPC((cmd, args) => {
  if (cmd === 'greet') return `Hello ${args.name}`
})
```

`mockIPC` intercepts `invoke()` calls; `mockWindows` fakes multi-window setups; `clearMocks` resets state. The `shouldMockEvents` option also swallows `plugin:event` traffic.

## Conventions

- Every module exposes a `@since` JSDoc on every public symbol — those are the versions that introduced the API.
- All modules return `Promise<T>` for any cross-IPC call.
- Each module's typedoc block in source code is the upstream reference.

# Examples

Located in [`repos-folder/tauri/examples/`](../../repos-folder/tauri/examples/). Most are minimal single-file demos runnable via `cargo run --example <name>` from the repo root; a few are full `src-tauri` projects.

| Example | Demonstrates | Run command |
| --- | --- | --- |
| `helloworld` | Smallest Tauri app — one `#[tauri::command]`, `Builder::default`, static `index.html`. | `cargo run --example helloworld` |
| `commands` | Many command shapes: sync, async (`async fn`), `#[command(async)]` futures, `State`, `Window`, `Request`/`Response`, custom errors, `rename_all`. | `cargo run --example commands` |
| `state` | Registering app state via `Builder::manage(T)` and reading it from commands with `State<'_, T>`. | `cargo run --example state` |
| `multiwindow` | Creating multiple windows from Rust, addressing them by label. | `cargo run --example multiwindow` |
| `multiwebview` | Multiple webviews inside one window (unstable feature). | `cargo run --example multiwebview --features unstable` |
| `drag` | Custom-decorated window drag handling. | `cargo run --example drag` |
| `splashscreen` | Show a splash window while the main UI / Rust setup is loading, then swap. | `cargo run --example splashscreen` |
| `run-return` | Using `Builder::run_return` to keep the runtime alive after the main loop exits — useful for embedding. | `cargo run --example run-return` |
| `streaming` | Custom URI scheme protocol that streams video via range requests. Note: Tauri already ships the `asset:` protocol for this; the example is reference material. | `cargo run --example streaming` |
| `isolation` | The Isolation pattern — IPC mediated by a sandboxed isolation webview. Set in `tauri.conf.json` `app.security.pattern.use = "isolation"`. | `cargo run --example isolation --features isolation` |
| `resources` | Bundling extra assets via `bundle.resources` and looking them up at runtime with `App::path_resolver()`. Needs a full setup; see its README. | full `src-tauri` build |
| `file-associations` | Registering app as the default handler for file extensions + custom UTIs on macOS. PNG/JPG/GIF + custom `taurid`/`taurijson`. | `cargo build --features tauri/protocol-asset` then run |
| `api` | The integration test / kitchen-sink app exercising the full `@tauri-apps/api` surface. Real `src-tauri` project with a Svelte/UnoCSS frontend. | see its README |

## Picking one as a starting point

- For a new app: start with `helloworld`, then look at `commands` and `state` for IPC patterns.
- For multi-surface UIs: `multiwindow` for separate windows, `multiwebview` for embedded webviews.
- For sandboxing: `isolation` shows the runtime-enforced IPC mediation.
- For bundling extra files: `resources` + the path resolver pattern is the canonical example.

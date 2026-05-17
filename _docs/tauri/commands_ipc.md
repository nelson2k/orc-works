# Commands & IPC

The Rust ↔ JS boundary in Tauri is a message-passing IPC. The frontend calls `invoke('cmd_name', args)`, the backend handler returns a serialized result. Events and channels add long-lived bidirectional traffic on top.

## Defining a command in Rust

```rust
#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello {name}, You have been greeted from Rust!")
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![greet])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
```

That's the `examples/helloworld/main.rs` form. The `#[tauri::command]` macro lives in `tauri-macros`; `generate_handler!` wires the list of commands into the runtime's invoke router.

## Command argument types

From `examples/commands/main.rs`, commands can accept:

- Plain typed args: `name: &str`, `the_argument: String`, custom `serde::Deserialize` structs.
- `State<'_, T>` — globally registered state via `Builder::manage(T)`.
- `Window` / `Webview` / `AppHandle` — runtime handles for the calling surface.
- `Request` / `Response` — `tauri::ipc::Request`/`Response` for raw bytes / non-JSON payloads.

Variants:

- `#[command(async)]` — turns a sync function whose return is a `Future` into an async command.
- `#[command(rename_all = "snake_case")]` — controls how the frontend's camelCase keys map to Rust args.
- `Result<T, E>` returns serialize errors back to JS as a rejected promise.

## Defining the call site in JS

```ts
import { invoke } from '@tauri-apps/api/core'

const greeting = await invoke<string>('greet', { name: 'world' })
```

The `core` module also exports `Channel`, `addPluginListener`, `PermissionState`, `checkPermissions`, `requestPermissions`, `convertFileSrc`, `isTauri`, and a `SERIALIZE_TO_IPC_FN` symbol for custom IPC serialization of class instances.

## IPC payload types

From `crates/tauri/src/ipc/mod.rs`:

```rust
pub enum InvokeBody {
    Json(JsonValue),
    Raw(Vec<u8>),
}
```

`InvokeBody::Raw` is **not supported on Android** — the enum is always `Json` there. For Android, pass raw bytes as a base64 string, not as a JSON number array (still more efficient).

## Events

`Window::emit("event", payload)` from Rust ↔ `listen("event", handler)` from JS (`@tauri-apps/api/event`). Events are fire-and-forget, broadcast-style; if you need ordered, point-to-point streaming, use a `Channel`.

## Channels

`tauri::ipc::Channel` (re-exported from `tauri::ipc`) is a unidirectional Rust→JS stream. The JS side constructs `new Channel()`, passes its id as a command argument, and Rust calls `.send(message)` repeatedly. The JS-side `Channel` class buffers and reorders messages by index so out-of-order delivery is reassembled (`packages/api/src/core.ts`).

## ACL — permissions & capabilities (v2)

v2 replaces v1's `allowlist` with a structured permission system:

- A **plugin** ships `permissions/*.toml` declaring identifiers like `core:event:default`.
- A **capability** is an app-side JSON/TOML file granting one or more permissions to a window or webview label.
- At compile time, `tauri-codegen` resolves capabilities + plugin permissions into a `RuntimeAuthority` (`crates/tauri/src/ipc/authority.rs`) — that's the runtime gatekeeper for invokes.

CLI helpers:
- `tauri permission new` / `tauri permission ls`
- `tauri capability new` / `tauri capability ls`
- `tauri inspect` — combined view of resolved ACL state.

`feature = "dynamic-acl"` (default) enables `Manager::add_capability` so apps can grow capabilities at runtime via `CapabilityBuilder`.

## Isolation pattern

If `app.security.pattern.use = "isolation"`, the IPC handshake is mediated by a sandboxed iframe loaded from `options.dir`. Every invoke from the main webview is intercepted by the isolation webview, which can mutate or reject the payload before it reaches Rust. The `isolation` Cargo feature enables the runtime side; example: [`examples/isolation`](../../repos-folder/tauri/examples/isolation).

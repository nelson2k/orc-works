# Plugins

A Tauri plugin is the standard extension point. From `ARCHITECTURE.md`, a plugin generally does three things:

1. Provides Rust code that does "something" useful.
2. Provides interface glue to make it easy to drop into an app.
3. Provides a JS API for calling that Rust code.

## Anatomy

A plugin crate depends on `tauri-plugin` (`crates/tauri-plugin`) with one or both features:

- `build` — for the plugin's `build.rs`. Exposes helpers for parsing permissions, generating the ACL manifest, validating schemas.
- `runtime` — for the plugin's runtime crate.

It usually ships alongside a JS package that wraps `invoke('plugin:<name>|<command>', args)` with a typed surface.

## The `Plugin` trait

From `crates/tauri/src/plugin.rs` — the runtime-side contract:

```rust
pub trait Plugin<R: Runtime>: Send {
    fn name(&self) -> &'static str;
    fn initialize(&mut self, app: &AppHandle<R>, config: JsonValue) -> Result<(), Box<dyn Error>>;
    fn initialization_script(&self) -> Option<String>;     // injected before HTML parse
    fn initialization_script_2(&self) -> Option<InitializationScript>;
    // hooks: on_navigation, on_page_load, on_webview_ready, on_event, extend_api, ...
}
```

`config` is the plugin's slice from `tauri.conf.json` under `plugins.<name>`.

The high-level helper is `Builder::new(&str)` (in `crates/tauri/src/plugin.rs` further down) which lets a plugin author declare commands, setup callbacks, init scripts, and event hooks without implementing the trait by hand.

## Permissions

Each plugin ships TOML files under its `permissions/` directory declaring identifiers like:

- `core:event:default`, `core:event:allow-listen`
- `dialog:allow-open`, `dialog:default`

The build script aggregates these into a manifest the host app's `tauri-codegen` step consumes when resolving capabilities. See [commands_ipc.md](commands_ipc.md#acl--permissions--capabilities-v2).

## Mobile plugins

`crates/tauri/src/plugin.rs` exposes a `mobile` module (gated by `#[cfg(mobile)]`). Mobile plugins additionally ship:

- An Android library (`<plugin>/android/`) Kotlin/Java code, declared via `tauri::mobile_entry_point!`.
- An iOS library (`<plugin>/ios/`) Swift code, bound via `tauri::ios_plugin_binding!` (in `crates/tauri/src/lib.rs`).

The CLI commands `tauri plugin android` and `tauri plugin ios` scaffold these per-OS subprojects (see `crates/tauri-cli/src/plugin/`).

## Authoring entry points (CLI)

From `crates/tauri-cli/src/plugin/mod.rs`:

- `tauri plugin new <name>` — scaffold a brand-new plugin crate + JS package.
- `tauri plugin init` — initialize plugin support in an existing crate.
- `tauri plugin android init` / `tauri plugin ios init` — add the mobile sub-projects.

## Consuming a plugin in an app

```rust
fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_sql::Builder::default().build())
        .run(tauri::generate_context!())
        .unwrap();
}
```

The CLI's `tauri add <plugin>` does this addition mechanically: adds the Cargo dep, the npm dep, and a stub capability file granting the plugin's `default` permission set.

## Examples of upstream plugins

`ARCHITECTURE.md` lists:

- <https://github.com/tauri-apps/tauri-plugin-sql>
- <https://github.com/tauri-apps/tauri-plugin-stronghold>
- <https://github.com/tauri-apps/tauri-plugin-authenticator>

(These live in their own repos, not in `repos-folder/tauri`.)

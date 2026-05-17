# CLI — `tauri` / `cargo tauri`

The CLI binary is `tauri-cli` in `crates/tauri-cli` (also wrapped as the napi `@tauri-apps/cli` package). It runs on macOS, Linux, and Windows.

## Subcommands

From `crates/tauri-cli/src/lib.rs` (the `Commands` enum):

| Command | What it does |
| --- | --- |
| `init` | Scaffold a `src-tauri` folder in an existing project. |
| `dev` | Build and run the app in dev mode. Drives the frontend dev server (or runs Tauri's built-in static server) and rebuilds the Rust side on change. |
| `build` | Compile the Rust binary in release mode and (by default) invoke the bundler. |
| `bundle` | Run only the bundler step on an existing build artifact. |
| `android` | Mobile (Android) subcommands — `init`, `dev`, `build`. |
| `ios` (macOS host only) | Mobile (iOS) subcommands. |
| `migrate` | Migrate a v1 project to v2 — config, manifest, plugin renames, permissions. |
| `info` | Dump environment details for triage: Node, Rust, system info, installed plugins. |
| `add` / `remove` | Install or uninstall an official plugin (Rust crate + JS package + ACL stub). |
| `plugin` | Author-side commands for plugins: `new`, `init`, `android`, `ios`. |
| `icon` | Generate platform icon sets from a single source PNG. |
| `signer` | Generate updater signing keys; sign artifacts. |
| `completions` | Emit shell completion scripts. |
| `permission` (ACL) | Generate / inspect plugin permissions. |
| `capability` (ACL) | Generate / inspect capability files. |
| `inspect` | Introspection tooling for ACL + config. |

Top-level flag: `-v / --verbose` (count, global). The verbosity can also be forced with `TAURI_CLI_VERBOSITY`.

## Config input format

The CLI accepts `--config` either as a JSON string (when the argument starts with `{`) or as a path. By extension:

- `.toml` — parsed as TOML.
- `.json5` — parsed as JSON5.
- anything else — parsed as JSON, with a JSON5 fallback (see `crates/tauri-cli/src/lib.rs`).

## Selected environment variables

Full list lives in `crates/tauri-cli/ENVIRONMENT_VARIABLES.md`. The ones most likely to matter:

- `CI` — non-interactive mode.
- `TAURI_CLI_CONFIG_DEPTH` — how many levels up to search for the config file.
- `TAURI_CLI_PORT` — port for the CLI's built-in dev server.
- `TAURI_CLI_WATCHER_IGNORE_FILENAME` — name of the per-directory ignore file used by the `dev` watcher.
- `TAURI_CLI_NO_DEV_SERVER_WAIT` — skip waiting for the frontend dev server.
- `TAURI_SIGNING_PRIVATE_KEY` / `TAURI_SIGNING_PRIVATE_KEY_PASSWORD` — updater signing key (string or path) and password.
- `TAURI_SIGNING_RPM_KEY` / `TAURI_SIGNING_RPM_KEY_PASSPHRASE` — GPG key for signing RPMs.
- `TAURI_WINDOWS_SIGNTOOL_PATH` — path to `signtool.exe` for Windows code signing.
- `APPLE_*` — code signing + notarization (`APPLE_CERTIFICATE`, `APPLE_ID`/`APPLE_PASSWORD`/`APPLE_TEAM_ID`, or `APPLE_API_KEY`/`APPLE_API_ISSUER`).
- `TAURI_ANDROID_PROJECT_PATH` / `TAURI_IOS_PROJECT_PATH` — path to the generated mobile projects (defaults to `<project>/src-tauri/gen/<android|ios>`).

## Hook command env vars

Set by the CLI when it runs `beforeDevCommand` / `beforeBuildCommand` / etc., useful for branching frontend builds:

- `TAURI_ENV_DEBUG` — `true` in `dev` or `build --debug`.
- `TAURI_ENV_TARGET_TRIPLE`, `TAURI_ENV_ARCH`, `TAURI_ENV_PLATFORM`, `TAURI_ENV_PLATFORM_VERSION`, `TAURI_ENV_FAMILY`.

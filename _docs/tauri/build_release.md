# Dev loop, release, updater

Mostly summarized from `ARCHITECTURE.md` plus the CLI source.

## Dev loop

```
pnpm tauri dev    # or yarn / npm
```

What happens, in order:

1. CLI reads `tauri.conf.json` (or `Tauri.toml` / `.json5`).
2. If `build.beforeDevCommand` is set, runs it (e.g. `pnpm dev`) — Tauri waits for `build.devUrl` to respond unless `TAURI_CLI_NO_DEV_SERVER_WAIT` is set.
3. Otherwise serves `build.frontendDist` via the CLI's built-in dev server on `TAURI_CLI_PORT`.
4. `cargo build` the Rust side. First build pulls and compiles a lot — subsequent rebuilds are incremental.
5. Opens a window with devtools enabled (`devtools` feature is on in debug builds by default).
6. Watches Rust sources; recompiles and relaunches the window on change. Frontend HMR is handled by your framework's dev server.

`pnpm tauri info` dumps environment + installed plugin versions for triage (`crates/tauri-cli/src/info/`).

## Release build

```
pnpm tauri build
```

1. Runs `build.beforeBuildCommand` (typically `pnpm build`) — the result must land in `build.frontendDist`.
2. `cargo build --release` with the `custom-protocol` feature on — Tauri assumes production mode and disables the dev server.
3. Hands the artifact to `tauri-bundler`, which produces installers for the host OS (see [bundler.md](bundler.md)).
4. Patches the binary's bundle-type marker so the running app knows which installer flavor produced it.
5. Outputs land under `./src-tauri/target/release/bundle/` (and the unbundled binary at `target/release/<binary>`).

Hook commands receive env vars (`TAURI_ENV_DEBUG`, `TAURI_ENV_TARGET_TRIPLE`, `TAURI_ENV_ARCH`, `TAURI_ENV_PLATFORM`, `TAURI_ENV_PLATFORM_VERSION`, `TAURI_ENV_FAMILY`) so the frontend build can branch by target.

No cross-compile. Use the `tauri-action` GitHub workflow if you need all-platform builds.

## Updater

The built-in updater is desktop only and driven by `bundle.updater` config in `tauri.conf.json`:

- `endpoints`: array of URL templates (with `{{target}}`, `{{arch}}`, `{{current_version}}` placeholders) the app polls for an `update.json` manifest.
- `pubkey`: ed25519 public key, generated alongside the private key by `tauri signer generate`.
- `dialog`: whether the updater shows its own UI or delegates to JS.

Flow:

1. Developer publishes signed artifacts + an `update.json` to the endpoint.
2. The running app fetches the manifest and compares versions.
3. If newer, the app downloads the artifact, verifies checksum **and** signature against the embedded pubkey.
4. The platform-specific install routine fires — for example NSIS / MSI on Windows runs the installer, `.AppImage` swaps the binary, `.dmg` mounts and replaces the bundle.
5. The app exits and is relaunched (when configured).

Signing inputs (env vars, summarized from `ENVIRONMENT_VARIABLES.md`):

- `TAURI_SIGNING_PRIVATE_KEY` — string or path.
- `TAURI_SIGNING_PRIVATE_KEY_PASSWORD` — passphrase.

The bundler emits an `updater_bundle` artifact alongside the regular installer per `bundle/updater_bundle.rs`. That's the file your update server points at.

## End-user flow

Users get a native installer for their OS and follow normal install/uninstall procedures. The binary itself is small because the OS WebView is reused; no Chromium / V8 / Node ship.

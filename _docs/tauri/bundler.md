# Bundler — `tauri-bundler`

The bundler turns a built Rust binary into platform-native installers and app packages. It lives at `crates/tauri-bundler` and can be used outside of a Tauri project (it began as a fork of `cargo-bundle`).

## What it produces (per host OS)

From `crates/tauri-bundler/src/lib.rs` and the per-OS modules under `src/bundle/`:

| Host OS | Output formats | Module |
| --- | --- | --- |
| macOS | `.app` bundle, `.dmg` | `bundle/macos/` — `app.rs`, `dmg/`, `icon.rs`, `ios.rs`, `sign.rs` |
| Linux | `.AppImage`, `.deb`, `.rpm` | `bundle/linux/` — `appimage/`, `debian.rs`, `rpm.rs`, `freedesktop/` |
| Windows | `.msi` (WiX), `.exe` (NSIS) | `bundle/windows/` — `msi/`, `nsis/`, `sign.rs` |
| All | `updater` artifact (signed) | `bundle/updater_bundle.rs` |

iOS bundling sits in `bundle/macos/ios.rs` since you can only build iOS apps from a macOS host.

**No cross-compile.** The bundler enforces host = target — e.g. `deb` is rejected when building on Windows. NSIS on Linux/macOS works only via `cargo-xwin`.

## Binary patching

The bundler patches the produced binary with a 32-byte marker `__TAURI_BUNDLE_TYPE_VAR_UNK`, replacing it with `__TAURI_BUNDLE_TYPE_VAR_<KIND>` (DEB, RPM, APP, NSS, MSI). This lets the running binary identify which installer flavor produced it — used by the updater to pick the right artifact (`crates/tauri-bundler/src/bundle.rs`).

macOS `.app` and `.dmg` are skipped for patching (the marker stays untouched).

## Signing

- **Updater signing**: ed25519 keys generated via `tauri signer generate`. Env vars `TAURI_SIGNING_PRIVATE_KEY`, `TAURI_SIGNING_PRIVATE_KEY_PASSWORD`.
- **macOS code signing + notarization**: `APPLE_CERTIFICATE`, `APPLE_ID`/`APPLE_PASSWORD`/`APPLE_TEAM_ID`, or alternatively `APPLE_API_KEY`/`APPLE_API_ISSUER`. The helper crate is `tauri-macos-sign`.
- **Windows code signing**: `TAURI_WINDOWS_SIGNTOOL_PATH` points at `signtool.exe`; the bundler signs via `sign.rs`.
- **RPM GPG signing**: `TAURI_SIGNING_RPM_KEY` (ASCII-armored), `TAURI_SIGNING_RPM_KEY_PASSPHRASE`.

## Settings

`bundle::settings::Settings` is the in-process knob bundle. It is populated from `bundle: { ... }` in `tauri.conf.json` — `targets`, `icon`, `fileAssociations`, plus the per-OS sub-blocks (`bundle.windows.wix`, `bundle.windows.nsis`, `bundle.macOS.signingIdentity`, etc.). The CLI's `build` command stitches `Settings` from the resolved config and hands it to `bundle::bundle_project`.

## Invocation

Via the CLI: `tauri build` (compiles + bundles) or `tauri bundle` (bundles only, presumes a build artifact exists).

As a library: `tauri_bundler::bundle::bundle_project(settings)` — returns `Vec<PathBuf>` of produced artifacts.

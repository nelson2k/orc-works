# Building on Windows

Source of truth: `docs/msw/install.md` in the upstream repo.

It is strongly recommended to define `WXWIN` as the env var pointing at the
checkout root, and to **avoid spaces** in that path (so not under
"C:\Program Files").

## Visual Studio (IDE)

Solutions live in `build\msw\`:

- `wx_vc14.sln` → MSVS 2015
- `wx_vc15.sln` → 2017
- `wx_vc16.sln` → 2019
- `wx_vc17.sln` → 2022
- `wx_vc18.slnx` → 2026

Pick configuration (Debug / Release / "DLL Debug" / "DLL Release") and a
platform (Win32 / x64 / ARM64). DLL builds may need to be rebuilt up to
three times due to project-order issues.

## Visual Studio (command line, MSBuild)

From a "Visual Studio Command Prompt" at `%WXWIN%\build\msw`:

```
msbuild /m /p:Configuration=Debug /p:Platform=x64 wx_vc17.sln
```

Quote configurations containing spaces: `/p:Configuration="DLL Release"`.

## Visual Studio (nmake, legacy)

```
nmake /f makefile.vc
```

Variables: `BUILD=release`, `SHARED=1`, `TARGET_CPU=X86|X64|ARM64`,
`UNICODE=1`, `VENDOR=...`, `CFG=...`. `nmake /a` forces full rebuild.
For incremental clean: `nmake /f makefile.vc clean`.

## MinGW / MinGW-w64 / TDM-GCC

Two paths:

1. **configure + make** (preferred) inside MSYS / MSYS2 / Cygwin:
   ```bash
   cd $WXWIN && mkdir build-debug && cd build-debug
   ../configure --enable-debug
   make
   cd samples/minimal && make
   ```
2. **`makefile.gcc`** in `build\msw\`, used without a Unix shell:
   ```
   mingw32-make -f makefile.gcc BUILD=release SHARED=1
   ```

## vcpkg

```
vcpkg install wxwidgets:x64-windows
```

## Verifying

Build the minimal sample (`samples/minimal/`) with the same parameters and
run it.

## Customizing

Copy `build\msw\wx_setup.props` → `wx_local.props` and edit the local copy.
That file overrides defaults (`wxCompilerPrefix`, `wxCfg`, `wxVendor`) and
won't be clobbered by upstream updates — but you must monitor for upstream
changes yourself.

For debugger visualizers: `misc/msvc/wxWidgets.natvis`.

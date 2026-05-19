# Libraries

All under `lib/gcc_x64_lib/` as static archives (`.a`).

| File | Target | Purpose |
|---|---|---|
| `wxmsw33u.a` | `wx::wxmono` | Monolithic core (base + core + adv + html + xrc + …). One archive holds everything because `MONOLITHIC=1`. |
| `wxregexu.a` | `wx::wxregex` | Internal regex engine (third-party). |
| `wxzlib.a` | `wx::wxzlib` | zlib copy bundled with wx. |
| `wxexpat.a` | `wx::wxexpat` | expat XML parser used by XRC. |
| `wxjpeg.a` | `wx::wxjpeg` | libjpeg copy. |
| `wxpng.a` | `wx::wxpng` | libpng (links wxzlib). |
| `wxtiff.a` | `wx::wxtiff` | libtiff (links wxzlib + wxjpeg). |
| `wxwebp.a` | `wx::webp` | libwebp encoder/decoder (links sharpyuv). |
| `wxwebpdemux.a` | `wx::webpdemux` | WebP demuxer. |
| `wxsharpyuv.a` | `wx::sharpyuv` | YUV utility used by libwebp. |
| `wxscintilla.a` | `wx::wxscintilla` | Scintilla source-editor backend (for `wxStyledTextCtrl`). |
| `wxlexilla.a` | `wx::wxlexilla` | Lexilla lexers (Scintilla companion). |

Because the build is **monolithic**, there is no separate `wxbase`,
`wxcore`, `wxhtml`, `wxaui`, etc. archive — they're all rolled into
`wxmsw33u.a`, exposed as the single target **`wx::mono`** (alias of
`wx::wxmono`). The per-component target aliases (`wx::core`, `wx::base`,
`wx::html`, etc.) are **not** created by `wxWidgetsConfig.cmake` for
this install, because the config only creates a `wx::<name>` alias when
the corresponding `wx::wx<name>` target exists in the targets file —
and for monolithic builds only `wx::wxmono` (plus image/regex/expat
helpers and `wx::wxbase_only`) is present.

Downstream code must link **`wx::mono`** (or `wx::wxmono` directly), not
`wx::core`. Requesting `find_package(wxWidgets CONFIG REQUIRED
COMPONENTS core base)` will fail `check_required_components`; use
`COMPONENTS mono` instead.

System libraries that `wx::wxmono` brings in via
`INTERFACE_LINK_LIBRARIES` (Windows-side):

```
kernel32 user32 gdi32 gdiplus msimg32 comdlg32 winspool winmm
shell32 shlwapi comctl32 ole32 oleaut32 uuid rpcrt4 advapi32
version ws2_32 wininet oleacc uxtheme winhttp imm32
opengl32 glu32   (wx::wxgl path; gated by OpenGL::GL)
```

Plus the bundled image/compression libraries above as `LINK_ONLY`
dependencies, and `Threads::Threads` from `find_dependency(Threads)`.

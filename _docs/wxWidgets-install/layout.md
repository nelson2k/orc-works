# Layout

```
wxWidgets-install/
├── bin/
│   └── wxrc.exe                      XRC resource compiler
├── include/
│   └── wx/                           ~424 public headers + subdirs (aui, msw, …)
├── lib/
│   ├── cmake/
│   │   └── wxWidgets-3.3/
│   │       ├── wxWidgetsConfig.cmake
│   │       ├── wxWidgetsConfigVersion.cmake
│   │       └── gcc_x64_lib/
│   │           ├── wxWidgetsTargets.cmake
│   │           └── wxWidgetsTargets-release.cmake
│   └── gcc_x64_lib/
│       ├── mswu/                     setup.h + build.cfg for release build
│       │   ├── build.cfg
│       │   └── wx/setup.h
│       ├── mswud/                    (empty — debug build not installed)
│       ├── wxmsw33u.a                monolithic core wx library
│       ├── wxregexu.a
│       ├── wxzlib.a
│       ├── wxexpat.a
│       ├── wxjpeg.a
│       ├── wxpng.a
│       ├── wxtiff.a
│       ├── wxwebp.a
│       ├── wxwebpdemux.a
│       ├── wxsharpyuv.a
│       ├── wxscintilla.a
│       └── wxlexilla.a
└── share/
    └── locale/                       46 locale folders, each with LC_MESSAGES/
```

Tag-named directories:

- `gcc_x64_lib` — the architecture tag for this install. Built with **gcc**,
  **64-bit**, **static** (`_lib` suffix; shared would be `_dll`).
- `mswu` — the toolkit+Unicode release flavor. `m` = MSW, `s` = static-crt
  (note: this install uses dynamic CRT despite the name — the `s` here is
  historical wx convention), `w` = wide/Unicode build, `u` = Unicode.
  `mswud` would be the debug variant; it's empty here.

`wxWidgetsConfig.cmake` picks the right `gcc_x64_lib` subfolder
automatically based on the consuming compiler and bitness.

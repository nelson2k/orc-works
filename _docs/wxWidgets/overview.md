# Overview

wxWidgets is a free, open-source, cross-platform **C++ GUI framework** that
uses native controls on each platform. It also abstracts non-GUI things
(threads, sockets, file I/O, streams, etc.).

## Version

`include/wx/version.h` declares:

- `wxMAJOR_VERSION 3`
- `wxMINOR_VERSION 3`
- `wxRELEASE_NUMBER 3`
- `wxVERSION_STRING "wxWidgets 3.3.3"`

Odd minor numbers (e.g. 3.3) are development releases; even minors (3.2) are
stable.

## Primary platforms

- **Windows** 7 / 8 / 10 / 11 — 32-bit and 64-bit Intel, plus ARM64
- **Unix** with GTK+ 2.6+, or GTK 3.x
- **macOS** 10.10+ on amd64 and ARM (Cocoa backend)

Additional ports exist (Qt, DFB, X11 direct, Wine, iOS, VMS) but are less
prominent — see [repo-layout.md](repo-layout.md).

## Compilers

C++11 minimum. Confirmed compilers per `README.md`:

- MSVC 2015 → 2026
- g++ 4.8 → 15 (MinGW / MinGW-w64 / TDM on Windows)
- Clang up to 19 / Xcode 16

For C++98 or Windows XP support, the 3.2 branch must be used instead.

## License

A modified LGPL that explicitly allows statically linking into a closed
application without releasing application sources. See
[license.md](license.md).

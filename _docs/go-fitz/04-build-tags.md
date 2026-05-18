# Build Tags

go-fitz uses Go build tags to swap between three linking strategies and several platform variations. The tags compose.

## Tags

- `extlib` - link against an externally installed MuPDF instead of the bundled static library. Without this, the bundled `libs/libmupdf_<os>_<arch>.a` is linked.
- `static` - meaningful only with `extlib`. Adds `-lmupdf-third` so a static system MuPDF that bundles third-party libraries works.
- `pkgconfig` - meaningful only with `extlib`. Replaces hand-rolled `LDFLAGS` with `pkg-config --libs mupdf`.
- `musl` - meaningful only without `extlib`. Selects the `_musl.a` variants under `libs/` (Linux amd64 / arm64 only).
- `nocgo` - switch to the experimental purego implementation. Works with `CGO_ENABLED=0`. Requires a system `libmupdf` shared library at runtime.

## Compile-time matrix

The two source files that contain the implementation are gated by:

```
fitz_cgo.go         //go:build cgo && !nocgo
fitz_nocgo.go       //go:build !cgo || nocgo
```

The three LDFLAGS helper files are gated by:

```
fitz_cgo_cgo.go              //go:build cgo && !nocgo && !extlib
fitz_cgo_extlib.go           //go:build cgo && !nocgo && extlib && !pkgconfig
fitz_cgo_extlib_pkgconfig.go //go:build cgo && !nocgo && extlib && pkgconfig
```

Exactly one of these helpers is compiled in any given build that uses cgo.

## Common combinations

```
go build                              # bundled static MuPDF (default)
go build -tags extlib                 # link system MuPDF, no pkg-config
go build -tags "extlib pkgconfig"     # link system MuPDF via pkg-config
go build -tags "extlib static"        # link static system MuPDF + third-party
go build -tags musl                   # bundled musl static MuPDF (Linux only)
go build -tags nocgo                  # purego, dlopen libmupdf at runtime
CGO_ENABLED=0 go build -tags nocgo    # same, with cgo disabled
```

## Runtime requirements per combination

- Bundled cgo build - none beyond a C compiler at build time (Windows additionally needs `-lcomdlg32 -lgdi32`; Android adds `-llog`).
- `extlib` cgo build - system MuPDF (headers and library) at build time, MuPDF shared library at runtime if dynamic.
- `nocgo` build - `libmupdf.so` / `libmupdf.dll` / `libmupdf.dylib` and `libffi` at runtime, plus a matching `FzVersion` / `FZ_VERSION`.

## Platform support of the bundled libraries

`libs/` ships static `.a` files for:

- `android_arm64`
- `darwin_amd64`, `darwin_arm64`
- `linux_amd64`, `linux_arm64`, plus `_musl` variants
- `windows_amd64`, `windows_arm64`

Other targets must use `extlib` or `nocgo`.

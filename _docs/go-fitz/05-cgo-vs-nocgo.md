# cgo vs nocgo Implementations

The package ships two implementations of the same public API: a cgo-based one and a purego one.

## cgo (default)

File: `fitz_cgo.go` (599 lines).

Calls MuPDF directly through `import "C"`. The preamble defines four small C wrappers that go through `fz_try` / `fz_catch` so MuPDF setjmp-style errors become regular `NULL` returns:

```c
fz_document *open_document(fz_context *ctx, const char *filename);
fz_document *open_document_with_stream(fz_context *ctx, const char *magic, fz_stream *stream);
fz_page     *load_page(fz_context *ctx, fz_document *doc, int number);
int          run_page_contents(fz_context *ctx, fz_page *page, fz_device *dev, fz_matrix transform, fz_cookie *cookie);
```

`run_page_contents` returns `1` on success, `0` on a fz exception - all per-page methods check for `0` and return `ErrRunPageContents`.

Pixel data is moved into Go memory with `C.GoBytes(pixels, 4*bbox.x1*bbox.y1)` and copied into an `image.RGBA`.

## nocgo (purego)

File: `fitz_nocgo.go` (1097 lines).

Uses `github.com/ebitengine/purego` and `github.com/jupiterrider/ffi` to load `libmupdf` at runtime and call each function via libffi-style trampolines. The same `Document` struct exists, but the unexported fields hold `uintptr`-backed Go types (`fzContext`, `fzDocument`, `fzStream`) rather than C pointers.

Each MuPDF symbol becomes a function variable populated at init time by `procAddress(handle, "fz_...")`.

Per-OS library loading lives in:

- `purego_linux.go` - `dlopen("libmupdf.so")` via `purego.Dlopen`.
- `purego_darwin.go` - `dlopen("libmupdf.dylib")`.
- `purego_windows.go` - `LoadLibrary("libmupdf.dll")` via `syscall.LoadLibrary`.

`FzVersion` (or `FZ_VERSION` env) is mandatory and must match the loaded library's `FZ_VERSION` string exactly; mismatches will manifest as garbled output or crashes.

## How the platform `.a` files are picked (cgo only)

`fitz_cgo_cgo.go` is a CGO-LDFLAGS-only file. It contains nothing but a long block of `#cgo` directives, one per platform/architecture/libc:

```
#cgo linux,amd64,!musl LDFLAGS: -L${SRCDIR}/libs -lmupdf_linux_amd64 -lmupdfthird_linux_amd64 -lm
#cgo linux,amd64,musl  LDFLAGS: -L${SRCDIR}/libs -lmupdf_linux_amd64_musl -lmupdfthird_linux_amd64_musl -lm
#cgo windows,amd64     LDFLAGS: -L${SRCDIR}/libs -lmupdf_windows_amd64 -lmupdfthird_windows_amd64 -lm -lcomdlg32 -lgdi32
... (etc, one entry per supported target)
```

The Go toolchain selects whichever line matches `GOOS / GOARCH / musl`. The `.a` files referenced live in `libs/`.

## Locking model

Both implementations share the same locking discipline: every `*Document` method that touches the underlying MuPDF context acquires `f.mtx` first. This means concurrent calls on the same document serialize. Different documents can be used in parallel from different goroutines because each has its own `*fz_context`.

There is no read/write split - even pure reads like `Text` take the lock.

## What changes between the two implementations

- Performance: cgo path is faster for many small operations because each purego call goes through libffi.
- Deployability: cgo requires a C toolchain at build time; nocgo requires a libmupdf shared library at deploy time.
- Error surface: identical; both return the same sentinel errors.
- API shape: identical, including method signatures and types returned.

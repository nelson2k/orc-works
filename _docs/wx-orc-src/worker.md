# Worker

`Worker.h` / `Worker.cpp` — spawns and talks to the Python OCR worker.
Windows-only; the `#else` branch errors out at compile time.

## Public API

```cpp
nlohmann::json request(const nlohmann::json& req,
                       ProgressCallback onProgress = nullptr);
void shutdown();
```

`request` is fully synchronous from the caller's point of view: it blocks
until the worker returns a non-progress reply. Concurrent calls serialize
on `mu_`. The intent is that callers run `request` on a background thread
and use `wxFrame::CallAfter` to ferry results back to the UI.

## Process startup (`ensureStarted`)

Lazy: the child is launched on the first `request`. Steps:

1. Create two anonymous pipes (`CreatePipe`), then disable inheritance
   on the parent-side ends with `SetHandleInformation`.
2. Open `NUL` for the child's stderr so worker chatter doesn't leak.
3. Build a `STARTUPINFOW` wired to the pipes, then `CreateProcessW` with
   `CREATE_NO_WINDOW`. The command line is built from `projectRoot()`,
   which calls `GetModuleFileNameW(nullptr, ...)` to get the exe path
   and returns `<exe-dir>\..\`:

   ```
   "<root>venv\Scripts\python.exe" "<root>worker.py"
   ```

   Since paths are anchored to the exe location, the working directory
   at launch time does not matter — see [layout.md](layout.md).
4. Allocate a 64 KiB `readBuf_` for line buffering.

## I/O

`writeLine` appends a `\n` if missing and writes via `WriteFile`.
`readLine` reads up to 64 KiB at a time into `readAccum_`, slices off the
next `\n`-terminated line, and trims a trailing `\r`. The accumulator
preserves bytes past the newline for the next call.

## Request loop

For each request, `Worker`:

1. Dumps the JSON object to one line and writes it.
2. Reads response lines one at a time:
   - `type:"progress"` → invoke `onProgress(resp)` and keep reading.
   - Anything else → return the parsed JSON.

Errors (failed write, EOF on stdout, parse failure) throw `std::runtime_error`.

## Shutdown

`shutdown` sends a single `{"cmd":"quit"}` line, closes stdin, waits up
to 3 s for the process to exit (`WaitForSingleObject`), and closes the
remaining handles. The destructor calls `shutdown()`.

## Protocol summary

What the caller in [main.cpp](main.md) actually sends:

- `{"cmd":"render", "path":..., "page":..., "dpi":120}` — returns
  `png_base64`, `pages`, or `type:"error"`.
- `{"cmd":"ocr", "path":..., "page":..., "engine":...}` — streams
  `type:"progress"` events (`kind` in `image|stage|tqdm`), then returns
  the final reply with `text`, `saved_to`, `engine`.

The schema is owned by the Python worker; this side just relays JSON.

# Worker

`Worker.h` / `Worker.cpp` — spawns, talks to, and can cancel the Python
OCR worker. Windows-only; the `#else` branch errors out at compile time.

## Public API

```cpp
nlohmann::json request(const nlohmann::json& req,
                       ProgressCallback onProgress = nullptr);
void cancel();
void shutdown();
```

`request` is synchronous from the caller's point of view: it blocks
until the worker returns a non-progress reply. Concurrent calls
serialize on `mu_`. The intended pattern is to run `request` on a
background thread and use `wxFrame::CallAfter` to ferry results back to
the UI — see [main.md](main.md).

## Process startup (`ensureStarted`)

Lazy: the child is launched on the first `request`. Steps:

1. Create two anonymous pipes with `CreatePipe`, then strip
   inheritability from the parent-side ends.
2. Open `NUL` for the child's stderr so worker chatter doesn't leak.
3. Build a `STARTUPINFOW` wired to the pipes and call `CreateProcessW`
   with `CREATE_NO_WINDOW`. The command line is built from
   `projectRoot()`, which calls `GetModuleFileNameW(nullptr, ...)` and
   returns `<exe-dir>\..\`:

   ```
   "<root>venv\Scripts\python.exe" "<root>worker.py"
   ```

   Paths are anchored to the exe location, so the working directory at
   launch time does not matter — see [layout.md](layout.md).
4. Allocate a 64 KiB `readBuf_` for line buffering.

## I/O

`writeLine` appends a `\n` if missing and writes via `WriteFile`.
`readLine` reads up to 64 KiB at a time into `readAccum_`, slices off
the next `\n`-terminated line, and trims a trailing `\r`. The
accumulator preserves bytes past the newline for the next call.

## Request loop

Per call:

1. Dump the JSON object to one line, write it.
2. Read response lines:
   - `type:"progress"` → invoke `onProgress(resp)`, continue reading.
   - Anything else → return the parsed JSON.

Errors (failed write, EOF on stdout, parse failure) throw
`std::runtime_error`.

## Cancel (hard-stop)

`cancel()` is callable from any thread **without holding the request
mutex** — that's the whole point. It:

1. `TerminateProcess(process_, 1)` and `WaitForSingleObject(.., 1000)`.
2. Closes child stdin/stdout handles and the process handle.
3. Clears `started_` and `readAccum_`.

Closing the handles while another thread is in `ReadFile`/`WriteFile`
makes that call return FALSE — `readLine`/`writeLine` then return false,
`request()` throws `"worker closed stdout"` (or similar), and the
calling thread's catch block runs. The next `request()` calls
`ensureStarted()` again and a fresh worker process is spawned.

[main.cpp](main.md) sets `stopRequested_` before calling `cancel()` so
the resulting exception can be reported as "stopped" instead of as an
error dialog.

## Shutdown (clean)

`shutdown()` sends `{"cmd":"quit"}`, closes stdin, waits up to 3 s for
exit, and closes the remaining handles. The destructor calls it.

## Protocol summary

Sent by [main.cpp](main.md):

- `{"cmd":"render", "path":..., "page":..., "dpi":120}` — returns
  `png_base64`, `pages`, or `type:"error"`.
- `{"cmd":"ocr", "path":..., "page":..., "engine":...}` — streams
  `type:"progress"` events (`kind` in `image|stage|tqdm`), then returns
  the final reply with `text`, `saved_to`, `engine`.

The schema is owned by the Python worker; this side just relays JSON.

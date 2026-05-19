# Gotchas

Things that aren't obvious from a casual read.

## Stdout isolation in worker.py (Local mode only)

Native libs printf'ing on FD 1 would corrupt the JSON line protocol.
The worker's first move (when not in `--http` mode) is to `os.dup(1)`
into a private FD, then redirect real FD 1 to `NUL` and replace
`sys.stdout` with a `/dev/null`-equivalent
([worker.py:29-38](../../wx-ocr-src/worker.py#L29-L38)).

Implications:

- All protocol output goes through `_proto_stream` and `send()`. Never
  use plain `print` in worker.py from the engine code.
- HTTP mode **deliberately skips** the redirect — uvicorn needs real
  stdout for its logs.

## Two-stage pip install (Pillow pin clash)

`marker-pdf` pins `Pillow<11.0.0`; `mineru[pipeline]` pins
`Pillow>=11.0.0`. pip's resolver refuses both in one pass, so the
install is split:

1. `pip install -r requirements-1.txt` — marker + transformers + autoawq + torch
2. `pip install -r requirements-2.txt` — mineru + fastapi + uvicorn (bumps Pillow)

pip emits a compatibility warning for marker-pdf in stage 2 — it's
benign in practice (per the comment in
[requirements-1.txt](../../wx-ocr-src/requirements-1.txt)).

## autoawq / PytorchGELUTanh shim

`autoawq` (deprecated, pinned to `transformers<=4.51`) does
`from transformers.activations import PytorchGELUTanh` at import time.
That symbol was removed in `transformers 4.51+`.

The transformers VLM path shims it back in *before* triggering AWQ
kernel loading
([worker.py:645-658](../../wx-ocr-src/worker.py#L645-L658)):

```python
class _PytorchGELUTanh(_nn.Module):
    def forward(self, x):
        return _F.gelu(x, approximate="tanh")
_act.PytorchGELUTanh = _PytorchGELUTanh
```

Don't move the import order — the shim has to be installed before
`Qwen2_5_VLForConditionalGeneration` is imported.

## On-disk page cache

[`_cached_page_output`](../../wx-ocr-src/worker.py#L848-L864): if
`output/<pdf-stem>/page_NNNN.md` exists, `ocr()` returns it tagged
`engine: "cached"` **without re-running OCR**. The `.md` file IS the
cache key.

To force a re-extract, delete the file. To force a fresh extract of
the whole PDF, delete the directory. This makes Extract PDF resumable
after an interruption.

## pkill self-match trap (avoided)

The HTTP VLM cleanup uses `pkill llama-server`, NOT `pkill -f llama-server`
([worker.py:530-531](../../wx-ocr-src/worker.py#L530-L531),
[worker.py:582-584](../../wx-ocr-src/worker.py#L582-L584)). With `-f`,
the regex matches process *command line*, and a python invocation like
`python worker.py …llama-server…` (the worker's own command line, which
mentions paths containing `llama-server` once env vars are expanded)
would match — `pkill` would kill the worker itself. Plain `pkill
llama-server` matches process *name* only.

## UTF-8 vs cp1252 mojibake

orcgui consistently uses `wxString::FromUTF8` on data crossing the
worker boundary (text, status messages, file paths).
[main.cpp:435](../../wx-ocr-src/src/main.cpp#L435) builds the title bar
em-dash from its UTF-8 bytes `\xE2\x80\x94`; the temperature label uses
a literal `°` in a wide-string literal at
[main.cpp:413](../../wx-ocr-src/src/main.cpp#L413).

Don't replace these with cp1252 / Latin-1 string handling — the worker
emits UTF-8 in its JSON and the text area expects UTF-8 going in.

## Path semantics differ by mode

In Local mode the worker reads `path` as a Windows path. In Remote
mode the worker reads `path` as a path on the *server's* filesystem
(`/tmp/ocr-works-<basename>`). [`PathForWorker`](../../wx-ocr-src/src/main.cpp#L570-L603)
abstracts this away.

Switching backend resets `remoteUploadedFor_` so the next request will
re-upload
([main.cpp:611-613](../../wx-ocr-src/src/main.cpp#L611-L613)).

## Remote cancellation isn't real

Pressing Stop in Remote mode closes the WinHTTP request handle so
orcgui stops listening, but the FastAPI worker thread runs to
completion (engines have no cooperative-cancel hook). The next
request will queue behind it.
[Worker.cpp:566-576](../../wx-ocr-src/src/Worker.cpp#L566-L576),
[worker.py:1034-1039](../../wx-ocr-src/worker.py#L1034-L1039).

## VRAM interlock between engines

Only one of `{_marker_models, _vlm}` should be resident at a time on a
small card. Every engine entry point unloads the other before loading
itself ([worker.py:265-272](../../wx-ocr-src/worker.py#L265-L272),
[worker.py:642-643](../../wx-ocr-src/worker.py#L642-L643),
[worker.py:525](../../wx-ocr-src/worker.py#L525)). MinerU additionally
unloads both before running.

`_marker_text_is_bad` MUST be called *before* the Auto-mode fallback
unloads Marker — the OCR error predictor lives inside `_marker_models`.
The dispatch order in `ocr()` respects this.

## Worker startup path layout

`Worker.cpp` assumes the exe lives at `<root>/build/orcgui.exe` and
the venv at `<root>/venv/Scripts/python.exe`
([Worker.cpp:22-31](../../wx-ocr-src/src/Worker.cpp#L22-L31),
[Worker.cpp:287-289](../../wx-ocr-src/src/Worker.cpp#L287-L289)). Move
the exe somewhere else and Local mode breaks.

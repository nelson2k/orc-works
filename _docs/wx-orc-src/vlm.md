# VLM engine — two implementations

[`ocr_vlm`](../../wx-ocr-src/worker.py#L755-L758) picks one of two
backends at request time:

```python
def ocr_vlm(path, page):
    if _vlm_use_http():
        return _ocr_vlm_http(path, page)
    return _ocr_vlm_transformers(path, page)
```

## Selection rule

[`_vlm_use_http`](../../wx-ocr-src/worker.py#L498-L504) checks three
files exist:

- `LLAMA_SERVER_BIN`            — default `/usr/local/bin/llama-server`
- `VLM_GGUF_DIR/Qwen2.5-VL-7B-Instruct-Q4_K_M.gguf`
- `VLM_GGUF_DIR/mmproj-Qwen2.5-VL-7B-Instruct-f16.gguf`

`VLM_GGUF_DIR` defaults to `<repo>/repos-folder/Qwen2.5-VL-7B-Instruct-GGUF`.
Override via env vars (`OCR_LLAMA_SERVER_BIN`, `OCR_VLM_GGUF_DIR`).

So in practice: on the remote 4070 with the GGUFs present → HTTP/llama-server
(7B Q4). On the Windows laptop in Local mode → transformers + AWQ (3B).

## HTTP backend (llama-server)

[`_ensure_vlm_http`](../../wx-ocr-src/worker.py#L518-L561):

1. `_vlm_http_ping()` to see if `127.0.0.1:8080/health` is already up
   (it might be from a previous worker run).
2. If not, unload Marker, `pkill llama-server` to clear any orphan,
   then `Popen` with `-ngl 99 -c 8192 --host 127.0.0.1 --port 8080`.
3. `start_new_session=True` so signals to the worker don't kill the
   server.
4. Poll `/health` up to 90s for readiness; if the subprocess exits early,
   throw.

Inference is an OpenAI-compatible chat completion with the image
inline as `data:image/png;base64,...` and
[`VLM_PROMPT`](../../wx-ocr-src/worker.py#L490-L495) — "transcribe …
as GitHub-flavored Markdown … Render math in LaTeX. Output only the
Markdown".

[`_unload_vlm_http`](../../wx-ocr-src/worker.py#L564-L584) terminates
the subprocess; if the server wasn't started by *this* process it
falls back to `pkill llama-server` (see [gotchas.md](gotchas.md) for
the `pkill -f` self-match trap that's deliberately avoided here).

## Transformers backend

[`_ensure_vlm_transformers`](../../wx-ocr-src/worker.py#L637-L683):

1. Unload Marker.
2. Apply a `PytorchGELUTanh` shim into `transformers.activations` —
   `autoawq` (deprecated, pinned to `transformers<=4.51`) imports this
   symbol at module load, but it was removed in `transformers 4.51+`.
   Re-implement as `nn.GELU(approximate="tanh")`
   ([worker.py:645-658](../../wx-ocr-src/worker.py#L645-L658)).
3. Load `Qwen2_5_VLForConditionalGeneration` from `OCR_VLM_PATH`
   (default `<repo>/repos-folder/Qwen2.5-VL-3B-Instruct-AWQ`), `torch_dtype="auto"`,
   `device_map="auto"`.
4. Build the `AutoProcessor` with `min_pixels=256·28·28` and
   `max_pixels=1280·28·28`.

[`_ocr_vlm_transformers`](../../wx-ocr-src/worker.py#L686-L740)
rasterizes the page at 200 DPI, runs `model.generate(max_new_tokens=4096,
do_sample=False)`, and decodes only the newly-generated tokens.

## Resident-model interlock

The two engines share VRAM. `_ensure_marker` calls `_unload_vlm`,
and both VLM ensure-functions call `_unload_marker`. The unload paths
zero the globals, `gc.collect()`, and call `torch.cuda.empty_cache()`
([worker.py:210-241](../../wx-ocr-src/worker.py#L210-L241)).

`atexit.register(_unload_vlm_http)` guarantees the llama-server
subprocess is terminated on a clean worker exit
([worker.py:752](../../wx-ocr-src/worker.py#L752)).

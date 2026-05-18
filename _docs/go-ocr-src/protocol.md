# Worker Protocol

The GUI sends one JSON command per line to the Python worker. The worker
returns one JSON object per line.

Commands:

```json
{"cmd":"render","path":"C:/file.pdf","page":0,"dpi":120}
{"cmd":"ocr","path":"C:/file.pdf","page":0,"engine":"auto"}
{"cmd":"quit"}
```

OCR engines accepted by `worker.py`:

- `auto`
- `digital`
- `marker`
- `marker_llm`
- `vlm`

Terminal responses:

```json
{"type":"image","page":0,"pages":10,"png_base64":"..."}
{"type":"text","engine":"marker","page":0,"text":"...","saved_to":"..."}
{"type":"error","message":"...","traceback":"..."}
```

Progress responses use `type:"progress"` and include a `kind`:

- `stage`: named pipeline stage
- `tqdm`: progress bar lifecycle or tick
- `image`: PNG overlay image to display in the GUI

The Go `Worker.request` method serializes requests with a mutex. It consumes
progress events until the first non-progress response, then returns that
terminal response to the caller.

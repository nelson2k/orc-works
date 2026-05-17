# Backends

Picked with `-b / --backend`. The three families are `pipeline`, `vlm`, and
`hybrid`; the latter two further split into `auto-engine` (local in-process)
and `http-client` (talk to a separate inference server).

| Backend                | Engine     | What it runs                                                     | Use when                                                          |
|------------------------|------------|------------------------------------------------------------------|-------------------------------------------------------------------|
| `pipeline`             | local      | 5 specialised models (layout + MFR + OCR + table cls/rec)        | CPU-only or low VRAM, want deterministic stack, no hallucinations |
| `vlm-auto-engine`      | local      | MinerU2.5-Pro VLM via vLLM / LMDeploy / mlx / transformers       | Have a GPU, want maximum accuracy on tough docs                   |
| `vlm-http-client`      | remote     | Same VLM, but client only — talks to a vLLM/LMDeploy/OpenAI server | Want a thin client; server lives elsewhere                       |
| `hybrid-auto-engine`   | local      | VLM + pipeline OCR (best text fidelity + VLM layout/tables)      | Default — best overall accuracy                                   |
| `hybrid-http-client`   | remote     | Same hybrid logic, but VLM half is remote                        | Server-fattening, low local VRAM but want hybrid quality          |

## Engine modes

- **`auto-engine`** loads the inference engine in-process. Means you need
  `mineru[pipeline]` / `mineru[vlm]` / `mineru[vllm]` etc. installed.
- **`http-client`** uses just `mineru[<lightweight>]` + `--url`. The remote
  server runs the heavy lift; the client only does pre/post-processing.
- For hybrid: even `hybrid-http-client` still needs **local pipeline deps**
  (because the pipeline's OCR runs locally). `ensure_backend_dependencies`
  in `cli/common.py` enforces this — if `torch` isn't importable, you get
  `HybridDependencyError` with a hint to install `mineru[pipeline]`.

## Trade-offs in one paragraph

`pipeline` is the workhorse — fast, runs on a CPU, deterministic, lower
accuracy ceiling but no hallucinations. `vlm` is the highest-quality option
on hard docs (math, tables, multi-column) but needs a real GPU. `hybrid` is
the new default and tries to get the best of both: the VLM handles layout
+ structure understanding while the pipeline OCR provides the actual text
characters (avoids the classic VLM hallucination problem on long passages).

## Picking a `-m / --method`

Only applies to `pipeline` and `hybrid-*` backends:

- `auto` — `pdf_classify` looks at the input bytes and decides if it's a
  scanned vs digital PDF, then routes accordingly. Default.
- `txt` — Trust the embedded text layer (pdftext / pypdfium2). Fast.
- `ocr` — Force OCR even if a text layer exists. Use for low-quality
  embedded text or font-encoding messes.

VLM backends ignore `-m` — the VLM always reads the page image.

## Picking a `-l / --lang`

Only matters for the pipeline-based OCR (PaddleOCR). VLM backends are
multilingual via the VLM itself and don't take `-l`. See [cli.md](cli.md)
for the full language list.

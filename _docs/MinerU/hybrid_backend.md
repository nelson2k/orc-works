# Hybrid backend (`mineru/backend/hybrid/`)

The current **default** (`hybrid-auto-engine`). Combines the VLM (for
layout understanding, table reconstruction, math/chart handling) with the
pipeline's PaddleOCR (for actual character recognition on text blocks).
The idea: get the VLM's structural smarts without paying for VLM
hallucination on long text passages.

## Why it exists

Pure VLM backends sometimes "summarize" long body text or substitute
plausible-but-wrong characters. The pipeline OCR is deterministic and
doesn't hallucinate. Running both means:

- VLM identifies regions, table grids, math, charts.
- For text regions, the **pipeline OCR** transcribes the actual pixels
  inside the VLM-identified box.
- Tables, math, and figures still flow through the VLM (where it wins).

Result: VLM-level structure with pipeline-level text fidelity.

## How it composes them

`hybrid_analyze.py` runs both engines per page in a coordinated loop:

1. `pdf_classify.classify` to decide whether OCR is even needed (some
   pages have perfect embedded text and skip OCR entirely).
2. `_load_hybrid_analyze_entrypoint` lazy-imports the hybrid module —
   `cli/common.py:ensure_backend_dependencies("hybrid-...")` enforces
   that local `torch` (pipeline deps) is installed even when the VLM
   itself is remote.
3. Page image → VLM via `MinerUClient` for layout + structure.
4. For each VLM-detected text box, `ocr_det` runs PaddleOCR detection +
   recognition (`PytorchPaddleOCR` from the pipeline backend).
5. MFR (math) is run for `Equation-Block` regions to get LaTeX.
6. `hybrid_magic_model.py` merges VLM output + OCR text + MFR LaTeX into
   the same `middle_json` schema.

Constants tuned for hybrid throughput (`hybrid_analyze.py`):

```
LAYOUT_BASE_BATCH_SIZE = 1
MFR_BASE_BATCH_SIZE = 16
OCR_DET_BASE_BATCH_SIZE = 8
```

These scale up with `batch_ratio` derived from `get_vram()`.

## Auto-engine vs http-client

- `hybrid-auto-engine` — VLM runs in-process via vLLM/LMDeploy/mlx/
  transformers. Pipeline OCR runs in-process.
- `hybrid-http-client` — VLM lives on a remote OpenAI-compatible server
  (saves the largest local memory chunk). Pipeline OCR **still runs
  locally** — you can't ship OCR over HTTP in this design.

## When to pick this backend

- It is the default for a reason — best overall accuracy on mixed-content
  docs.
- Hits a sweet spot on a 12+ GB GPU: VLM fits, OCR runs on the same card,
  text quality is high.
- For low-VRAM setups, use `hybrid-http-client` with a remote VLM server.

## When *not* to pick this

- No local GPU at all → use `pipeline` (CPU-friendly) or
  `vlm-http-client` (lightweight client).
- Wholly text-only digital PDFs where the pipeline's OCR is overkill →
  `pipeline -m txt` is faster and simpler.
- You need fully deterministic output → `pipeline` is the only fully
  deterministic backend.

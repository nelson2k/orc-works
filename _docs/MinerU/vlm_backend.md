# VLM backend (`mineru/backend/vlm/`)

The MinerU 2.5 vision-language model handling the whole document end-to-end.
Lives in `mineru.backend.vlm`. Calls into the third-party
`mineru_vl_utils.MinerUClient` (a pip dep) which speaks an OpenAI-compatible
chat protocol to whichever inference engine is loaded.

## The model

- **`MinerU2.5-Pro-2604-1.2B`** (current as of 3.1.0). 1.2B parameters.
- ~2.5 GB on disk. Fits comfortably on a 12+ GB GPU.
- Supports image + chart parsing, truncated-paragraph merge across pages,
  cross-page table merging, in-table image recognition.
- Outputs structured layout blocks (`BlockType` enum in
  `mineru_vl_utils.structs`).

## Two engine modes

### `vlm-auto-engine` (local in-process)

The CLI/server boots one of these depending on `mineru[...]` install and
platform:

| Engine        | Install          | Platform                 | Notes                                              |
|---------------|------------------|--------------------------|----------------------------------------------------|
| vLLM          | `mineru[vllm]`   | Linux                    | Highest throughput; recommended on multi-GPU.      |
| LMDeploy      | `mineru[lmdeploy]` | Windows (mostly)       | Tries to be the Windows equivalent of vLLM.        |
| mlx-vlm       | `mineru[mlx]`    | macOS (Apple Silicon)    | Uses MLX framework on M-series chips.              |
| transformers  | `mineru[vlm]`    | Anywhere                 | Fallback; slowest. Plain `AutoModelForCausalLM`.   |

Selection happens in `mineru/utils/engine_utils.py:get_vlm_engine` based on
which engines successfully import.

### `vlm-http-client` (remote)

Talks to a separate inference server via OpenAI-compatible REST. Spawned
with one of:

- `mineru-vllm-server`
- `mineru-lmdeploy-server`
- `mineru-openai-server` (proxy to a real OpenAI-API-compatible host, e.g.
  for a remote MinerU2.5 deployment or even a different VLM you've trained
  to follow the same prompt format)

The client side only needs `mineru` base + `mineru[vlm]` is *not* required
— good for thin Docker images.

## Per-document flow

1. PDF rendered to images via `pypdfium2`.
2. Each page image sent to the VLM through `MinerUClient.predict_image`
   (auto-engine) or `MinerUClient.predict_image_async` (http-client).
3. Model emits a structured response containing layout blocks with bboxes,
   labels, content (text / HTML for tables / LaTeX for math / mermaid for
   diagrams).
4. `vlm_magic_model.py` reshapes the VLM output into the same `middle_json`
   schema the pipeline backend produces (so downstream rendering is shared).
5. `vlm_middle_json_mkcontent.py` renders middle JSON → markdown / content
   list / images (same as pipeline).

## Concurrency

`vlm_analyze.py` has `aio_predictor_execution_guard` /
`predictor_execution_guard` to serialise / parallelise GPU work depending
on engine. vLLM and LMDeploy handle concurrency themselves; transformers
fallback uses the guards for back-pressure.

## When to pick this backend

- Have a 12+ GB GPU (or remote server with one).
- Documents are math-heavy, table-heavy, or have complex layouts.
- Multilingual without wanting to manage per-language OCR packs.
- Throughput matters more than CPU-friendliness — vLLM is fast.

Don't pick if you can't budget the VRAM, or if your docs are 99% plain
text in a single language (pipeline is faster and just as accurate there).

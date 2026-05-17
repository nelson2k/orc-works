# Chandra vs Marker

Both are Datalab projects; both convert documents to markdown/HTML/JSON. They
sit at opposite ends of the design spectrum.

| Aspect                   | Marker                                                  | Chandra                                                    |
|--------------------------|---------------------------------------------------------|------------------------------------------------------------|
| Architecture             | Long pipeline: 5 surya models + heuristics + opt. LLM   | One VLM (Qwen 3.5 VL-based) per page                       |
| Models                   | layout / detection / recognition / table-rec / ocr-error| `datalab-to/chandra-ocr-2` (one ~7B VLM)                   |
| LLM role                 | Optional cleanup (`--use_llm`, Gemini/Claude/OpenAI/etc)| **The whole model.** No external LLM, no per-stage tweaks. |
| Steps per page           | dozens (provider → builders → 20+ processors → render)  | one chat call → deterministic HTML→md/html/chunks          |
| Backends                 | Local torch only                                        | Local `hf` **or** remote `vllm`                            |
| Layout output            | Block tree (`Document` → `Page` → typed blocks)         | Flat list of top-level `<div data-bbox data-label>`        |
| Output formats           | markdown, html, json, chunks, ocr_json, extraction      | markdown, html, chunks (+ metadata)                        |
| Extensibility            | Subclass processors / renderers / providers / services  | Customize the prompt, that's it                            |
| Per-page cost (local)    | Small per stage, total adds up                          | One ~12k-token generation per page                         |
| Throughput (H100)        | Highly parallel via batched models                      | 1.44 pages/s @ 96 concurrent (vLLM)                        |
| Multilingual             | Decent for Latin scripts                                | Strong — 90+ languages, leads benchmarks                   |
| Math / tables / handwriting | Mediocre without LLM; LLM closes the gap             | Strong out of the box                                      |
| Code license             | GPL-3.0                                                 | Apache-2.0                                                 |
| Weights license          | Modified OpenRAIL-M                                     | Modified OpenRAIL-M                                        |

## When to pick which

**Marker is better when**:
- You want a deterministic, transparent pipeline you can debug and extend.
- You have CPU-only or low-VRAM hardware (the heuristic-only path actually
  works).
- You want to swap renderers (chunks for RAG, ocr_json for re-OCR data).
- Doc structure matters more than absolute OCR quality.

**Chandra is better when**:
- You have a GPU and care about raw quality, especially math/tables/forms.
- Multilingual coverage matters.
- You want one model with one config, not a 30-stage pipeline.
- Production throughput via vLLM is the deployment story.

## In this repo

- Marker source: `repos-folder/marker/`, docs: [_docs/marker/](../marker/README.md).
- Chandra source: `repos-folder/chandra/`, docs: this folder.
- Both are read-only references (per repo conventions in
  [_docs/instructions.txt](../instructions.txt)).

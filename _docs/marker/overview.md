# Overview

Marker is a pipeline of deep-learning models plus heuristics that turns
documents into structured markdown / JSON / HTML.

## Core abstractions (from `marker/`)

| Component  | Where             | Job                                                |
|------------|-------------------|----------------------------------------------------|
| Provider   | `providers/`      | Read a source file (PDF, image, …) → page bboxes, lines, refs. |
| Builder    | `builders/`       | Build the in-memory `Document`: layout, lines, OCR, grouping. |
| Processor  | `processors/`     | Mutate blocks (tables, equations, headers, footnotes, lists, …). |
| Renderer   | `renderers/`      | Serialize the final `Document` to a string format. |
| Service    | `services/`       | LLM backend (Gemini, Vertex, Claude, OpenAI, Azure, Ollama). |
| Converter  | `converters/`     | Glues a provider + builders + processors + renderer together. |
| Schema     | `schema/`         | Block classes (`Document`, `PageGroup`, `Text`, `Table`, …). |

## Hybrid mode

Pass `--use_llm` to enable LLM-backed processors (table merging across pages,
form extraction, inline-math fixups, image descriptions, etc.). The LLM service
defaults to `marker.services.gemini.GoogleGeminiService`.

## When models actually run

- `surya` is used for layout, line detection, OCR-error detection, OCR
  recognition, and table recognition.
- `texify` and other models referenced in the README are pulled in via surya.
- All neural models are constructed once via [models_and_output.md](models_and_output.md)
  (`create_model_dict`) and injected into builders that need them.

## Pipeline outputs

`text_from_rendered(rendered)` returns `(text, ext, images)` for the four main
renderer outputs — see [renderers.md](renderers.md).

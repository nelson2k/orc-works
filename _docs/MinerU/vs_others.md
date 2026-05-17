# MinerU vs marker / chandra / pdftext

All four turn documents into markdown/JSON for downstream LLM use. They
sit in different parts of the design space.

| Aspect                     | MinerU                                       | Marker                                  | Chandra                            | pdftext                       |
|----------------------------|----------------------------------------------|-----------------------------------------|------------------------------------|-------------------------------|
| Inputs                     | PDF, image, **DOCX, PPTX, XLSX**             | PDF, image, DOCX, XLSX, PPTX, EPUB, HTML | PDF, image                         | PDF only                      |
| Backends                   | pipeline / vlm / hybrid + http variants      | One pipeline (surya + heuristics + opt. LLM) | One VLM (Qwen 3.5 VL)              | pypdfium2 + heuristics        |
| LLM role                   | Hybrid or full VLM (in-house MinerU2.5-Pro)  | Optional cleanup (Gemini/Claude/OpenAI) | Whole model is the VLM             | None                          |
| Server / orchestration     | **mineru-api + mineru-router (multi-GPU)**   | Single-process or batch                 | Single-process or vLLM client      | Single-process / multi-process|
| Native office              | **Yes** (DOCX/PPTX/XLSX without PDF round-trip) | Via PDF round-trip (WeasyPrint)      | No                                 | No                            |
| 109-language OCR           | Yes (PaddleOCR packs)                        | ~Latin scripts                          | Strong via VLM                     | Embedded text only            |
| Local CPU usable           | `pipeline` backend yes; others need GPU      | Yes (CPU + 6 GB GPU min for useful run) | No (~14 GB VRAM minimum)           | Yes (CPU only)                |
| Code license               | Custom-based-on-Apache-2 (was AGPLv3)        | GPL-3.0                                 | Apache-2.0                         | Apache-2.0                    |
| Weights license            | Modified OpenRAIL-M                          | Modified OpenRAIL-M                     | Modified OpenRAIL-M                | N/A                           |
| Throughput target          | 200M+ pages/week (managed)                   | One-doc-at-a-time CLI                   | 1.44 pages/sec on H100             | Limited by single-process     |

## When to pick which

**Marker** (what you've been using): single-doc CLI, low VRAM-friendly,
fastest path to "PDF → markdown" on a 6 GB box. The right pick for
ad-hoc personal use.

**Chandra**: highest single-pass quality on tough docs (math, tables,
handwriting, multilingual), but needs a 16+ GB GPU. Production-ish via
its vLLM server.

**MinerU**: production parsing service across multiple GPUs, multiple
formats, multiple users. The right pick when you need a *parsing
platform* not a *parsing script*.

**pdftext**: building block, not an end-user tool. Use directly when you
just need text + bboxes + fonts from digital PDFs.

## Why this repo has all of them

`repos-folder/` is a reference shelf — none of them are installed into
the working venv (`marker-code/venv/`). When you decide MinerU's
hybrid-auto-engine is the right tool for a job, you'd:

1. Read `_docs/MinerU/` to confirm fit.
2. `pip install mineru[all]` into a separate venv.
3. Wire it into your `run.py` style script (or stand up `mineru-api` and
   point a client at it).

The four parsers can also compose: you could use **pdftext** to fast-path
text-only digital PDFs and fall back to **MinerU hybrid** for image-heavy
or tabular pages. Same fast-path / slow-path idea as the marker
orchestrator suggestion from earlier in this session.

## Honest take

For your `017-ocr-works` repo (book conversion on a 2060 / 4070):

- Stick with marker for the actual conversion.
- Read MinerU's hybrid-backend logic for inspiration — the
  "VLM-for-structure + OCR-for-text-fidelity" combo is the cleanest
  answer to VLM hallucination, and you could backport the idea to your
  marker pipeline if you ever want.
- If you scale beyond personal use, MinerU's API + router is the most
  production-ready of the bunch.

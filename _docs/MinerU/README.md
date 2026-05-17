# MinerU — Docs Index

MinerU (by OpenDataLab) is a production document-parsing engine that
converts PDF, image, DOCX, PPTX, and XLSX inputs into Markdown and JSON for
RAG / LLM / agent workflows. Originally AGPLv3; relicensed to a
custom-license-based-on-Apache-2.0 in 3.1.0 (2026/04).

Source repo: `repos-folder/MinerU/`.

Key features that distinguish it from marker/chandra:
- **Three parsing backends** (`pipeline`, `vlm`, `hybrid`) with two engine
  modes each (`auto-engine` local, `http-client` remote).
- **Native DOCX / PPTX / XLSX** parsing — not just PDF.
- **OpenAI-compatible API**, FastAPI server, Gradio WebUI, multi-GPU router.
- **109-language OCR**, formula → LaTeX, table → HTML, cross-page table
  merging, scanned-doc detection.

## Architecture at a glance

```
mineru CLI (orchestration client)
  └─ HTTP → mineru-api (FastAPI)         [auto-starts locally if --api-url omitted]
       └─ planned tasks → backend dispatch:
            ├─ pipeline backend  → 5 specialized models (layout, MFR, OCR, table cls/rec)
            ├─ vlm backend       → MinerU2.5-Pro-2604-1.2B VLM (vLLM / LMDeploy / mlx / transformers)
            └─ hybrid backend    → pipeline + VLM combined
       └─ middle JSON  → markdown / content list / images / visualizations
```

## Pages

- [overview.md](overview.md) — what MinerU is, module map
- [install.md](install.md) — pip extras (core / pipeline / vlm / vllm / lmdeploy / mlx / gradio)
- [cli.md](cli.md) — `mineru` command + all 7 entry points
- [backends.md](backends.md) — pipeline vs vlm vs hybrid, auto-engine vs http-client
- [pipeline_backend.md](pipeline_backend.md) — layout / MFR / OCR / table model stack
- [vlm_backend.md](vlm_backend.md) — MinerU2.5-Pro VLM, vLLM / LMDeploy / mlx servers
- [hybrid_backend.md](hybrid_backend.md) — combined VLM + pipeline pipeline
- [office.md](office.md) — native DOCX / PPTX / XLSX handling
- [server.md](server.md) — mineru-api, mineru-router, vLLM/LMDeploy/OpenAI servers
- [output.md](output.md) — middle JSON, markdown, content lists, visualizations
- [settings.md](settings.md) — `~/mineru.json` + env vars
- [integrations.md](integrations.md) — MCP / LangChain / RAGFlow / Dify
- [vs_others.md](vs_others.md) — vs marker, chandra, pdftext

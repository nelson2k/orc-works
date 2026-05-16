# Marker — Docs Index

Marker (by Datalab) converts PDF, images, PPTX, DOCX, XLSX, HTML, EPUB to
markdown / JSON / HTML / chunks. Code is GPL-3.0; model weights use a modified
AI Pubs Open Rail-M license.

Source repo: `repos-folder/marker/`.

## Pipeline at a glance

```
filepath
  └─ Provider (read source)
      └─ DocumentBuilder
          ├─ LayoutBuilder (surya layout)
          ├─ LineBuilder   (surya detection + ocr-error)
          └─ OcrBuilder    (surya recognition)
              └─ StructureBuilder (group captions, lists)
                  └─ Processor[] (heuristic + LLM)
                      └─ Renderer (markdown / json / html / chunks)
```

## Pages

- [overview.md](overview.md) — high-level architecture
- [pipeline.md](pipeline.md) — pipeline stages in order
- [converters.md](converters.md) — top-level orchestration classes
- [builders.md](builders.md) — document / layout / line / ocr / structure
- [providers.md](providers.md) — source-file readers (PDF, image, …)
- [processors.md](processors.md) — heuristic block processors
- [processors_llm.md](processors_llm.md) — LLM-backed block processors
- [renderers.md](renderers.md) — markdown / json / html / chunk / ocr_json
- [schema.md](schema.md) — `BlockTypes`, blocks, groups, document
- [services.md](services.md) — LLM backend services (Gemini, Claude, …)
- [extractors.md](extractors.md) — structured extraction (beta)
- [config.md](config.md) — CLI / config parsing
- [scripts.md](scripts.md) — CLI entry points and FastAPI server
- [models_and_output.md](models_and_output.md) — `create_model_dict`, output helpers
- [settings.md](settings.md) — env / runtime settings
- [extending.md](extending.md) — how to add your own processor / renderer / provider
- [usage.md](usage.md) — common CLI and Python recipes

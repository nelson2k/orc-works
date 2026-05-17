# Package layout

Contents of `repos-folder/docling/`:

```
docling/                       main Python package (the SDK + CLI)
├── __init__.py
├── document_converter.py      DocumentConverter — main entry point
├── document_extractor.py      DocumentExtractor — structured-extraction entry point (beta)
├── exceptions.py              ConversionError and other exception types
├── backend/                   file-format adapters (PDF, Office, HTML, MD, XML, audio, …)
├── chunking/                  re-exports from docling-core.transforms.chunker
├── cli/                       typer CLI (docling, docling-tools)
├── datamodel/                 pydantic types: options, settings, results, format enums
├── experimental/              in-progress models (e.g. TableCropsLayoutModel)
├── models/                    every ML model + factories + plugin defaults
├── pipeline/                  Simple / StandardPdf / Vlm / Asr / Extraction pipelines
├── service_client/            httpx/websockets client for remote KServe/Triton servers
├── utils/                     profiling, accelerator detection, deepseek-ocr parse helper, file utils
└── py.typed
packages/
├── docling/                   meta-package (workspace member) that pins docling-slim[standard]
└── docling-slim/              README + assets for the PyPI package
tests/                         pytest suite + reference-output fixtures
docs/                          mkdocs.yml-driven user documentation, examples, recipes
scripts/                       maintenance scripts
Dockerfile                     container image for the CLI
Makefile                       make setup / test / check / validate targets
pyproject.toml                 the only authoritative config — extras, scripts, tools
mkdocs.yml                     documentation site config
tach.toml                      tach (module-boundary linter) config
uv.lock                        uv lockfile
```

## Call graph (PDF, standard pipeline)

```
docling.cli.main:app  /  DocumentConverter.convert(...)
  └─ DocumentConverter._convert
       └─ DocumentConverter._process_document
            └─ DocumentConverter._execute_pipeline
                 └─ StandardPdfPipeline.execute
                      ├─ _build_document
                      │   └─ ThreadedPipelineStage(preprocess → ocr → layout → table → assemble)
                      │       ├─ PagePreprocessingModel
                      │       ├─ Ocr*Model (auto / easyocr / rapidocr / tesseract / ...)
                      │       ├─ LayoutModel (Heron / Egret / V2)
                      │       ├─ TableStructure(V1|V2|GraniteVision)Model
                      │       └─ PageAssembleModel
                      ├─ _assemble_document
                      │   └─ ReadingOrderModel  → conv_res.document
                      ├─ _enrich_document
                      │   ├─ DocumentPictureClassifier
                      │   ├─ PictureDescription{Api,Vlm,VlmEngine}Model
                      │   ├─ ChartExtractionModelGraniteVision(_V4)
                      │   └─ CodeFormulaVlmModel
                      └─ _unload  → close backends
```

## Top-level files in `repos-folder/docling/`

- `README.md` — upstream marketing/install/quickstart
- `CHANGELOG.md` — release notes
- `AGENTS.md` — instructions for AI coding agents working on docling itself
- `CLAUDE.md` — same content delegated to AGENTS.md
- `CITATION.cff` — citation metadata for the technical report
- `CODE_OF_CONDUCT.md`, `CONTRIBUTING.md`, `MAINTAINERS.md`
- `LICENSE` — MIT
- `Dockerfile`, `Makefile`
- `pyproject.toml`, `uv.lock`
- `mkdocs.yml`, `tach.toml`

## Notable external dependencies

The package leans heavily on sibling packages from the docling-project org:

- `docling-core` — `DoclingDocument`, `DocItem*`, chunkers, HTML/markdown serializers, `LayoutVisualizer`, schema definitions.
- `docling-parse` — IBM's PDF understanding library (the default PDF backend).
- `docling-ibm-models` — model weights and wrappers (Heron / Egret layout, TableFormer V1/V2, code-formula, page-classifier, …).

Together these form the actual brain; the `docling` package itself is mostly orchestration, option parsing, and adapter glue.

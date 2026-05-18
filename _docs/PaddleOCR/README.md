# PaddleOCR Folder Inventory

This document describes the contents of the `PaddleOCR` folder as inspected locally. It is a self-contained inventory of what is in that folder.

## Overview

`PaddleOCR` is a large OCR and document AI toolkit centered on PaddlePaddle. The folder contains Python packages, model and pipeline wrappers, training and inference tools, deployment examples, documentation, tests, benchmark material, browser SDK code, and integration examples.

The package metadata identifies it as:

- Project name: `paddleocr`
- Description: "Awesome multilingual OCR and document parsing toolkits based on PaddlePaddle"
- Python support: `>=3.8`
- License: Apache License 2.0
- Main runtime dependency: `paddlex[ocr-core]>=3.5.0,<3.6.0`
- Console command: `paddleocr`

The README presents it as a toolkit for converting PDFs and images into structured OCR/document outputs such as JSON and Markdown, with support for multilingual OCR, document parsing, layout/table/formula/seal recognition, document translation, and browser inference.

## Size And File Makeup

Top-level content includes 19 directories and several project files. The largest areas are documentation, source code, deployment examples, and tests.

Common file types found:

- 529 Python files
- 447 Markdown files
- 300 text files
- 233 YAML/YML files
- 168 JPG images
- 152 PNG images
- 78 TypeScript files
- 55 C/C++ header files
- 43 C++ `.cc` files
- 42 shell scripts
- 23 JSON files
- 20 Dockerfiles
- 18 font files

## Root Files

- `README.md`: Main project overview, feature list, release notes, and usage documentation entry point.
- `pyproject.toml`: Python packaging metadata, dependencies, optional extras, package discovery, and pytest config.
- `setup.py`: Minimal setuptools entry point that delegates to packaging metadata.
- `requirements.txt`: Additional Python requirements list.
- `LICENSE`: Apache 2.0 license text.
- `MANIFEST.in`: Packaging manifest.
- `mkdocs.yml` and `mkdocs-ci.yml`: Documentation site configuration.
- `awesome_projects.md`: List of projects/ecosystem references.
- `train.sh`: Shell helper for training.
- `.pre-commit-config.yaml`, `.style.yapf`, `.clang_format.hook`: Formatting and pre-commit configuration.
- `.gitignore`, `.lycheeignore`, `CNAME`: Repository configuration/support files.

## Top-Level Directories

| Directory | Contents |
| --- | --- |
| `.github` | GitHub workflow and repository automation files. |
| `applications` | Application-level README material. |
| `benchmark` | Benchmark scripts and supporting configs for OCR runs. |
| `configs` | Model/training configs split by OCR task: classification, detection, recognition, end-to-end OCR, key information extraction, super-resolution, and table recognition. |
| `deploy` | Deployment examples for multiple targets, including C++ inference, serving, Paddle Lite, Android, Docker, FastDeploy-style service assets, AVH, and slim/compression flows. |
| `doc` | Documentation assets such as language fonts. |
| `docs` | Large documentation tree for versioned user guides, algorithms, deployment guides, images, and tutorial pages. |
| `langchain-paddleocr` | Separate LangChain integration package with tests, README files, package metadata, and sample data. |
| `mcp_server` | MCP server package and README material for exposing PaddleOCR capabilities through an MCP-style interface. |
| `overrides` | MkDocs theme overrides, partial templates, and hooks. |
| `paddleocr` | Main modern Python package exposed by the `paddleocr` command. |
| `paddleocr-js` | Browser/JavaScript SDK code, including TypeScript sources and web packaging files. |
| `ppocr` | Core OCR training/inference internals: datasets, model components, losses, metrics, optimizer logic, post-processing, utilities, and extensions. |
| `ppstructure` | Document structure tools: table recognition, layout recognition, key information extraction, PDF-to-Word, and recovery/export helpers. |
| `readme` | Localized README files in multiple languages. |
| `skills` | Skill-style documentation or integration assets. |
| `test_tipc` | TIPC test scripts, configs, expected results, docs, and supplementary models/utilities for training/inference validation. |
| `tests` | Python test suite and sample input files. |
| `tools` | Training, evaluation, export, and inference scripts for OCR tasks. |

## Main Python Package: `paddleocr`

The `paddleocr` directory is the installable package selected by `pyproject.toml`. It contains the command-line entry point and high-level APIs.

Important pieces:

- `__main__.py`: Console entry for the `paddleocr` command.
- `_cli.py`, `_common_args.py`, `_utils/cli.py`: CLI argument and command helpers.
- `_models`: Wrappers for document VLM, image classification, object detection, text detection, text recognition, formula recognition, layout detection, table recognition, seal recognition, document orientation, text line orientation, text image unwarping, and related model tasks.
- `_pipelines`: Higher-level pipelines for OCR, PaddleOCR-VL, document preprocessing, document understanding, formula recognition, PP-StructureV3, PP-ChatOCRv4 document processing, document translation, seal recognition, and table recognition.
- `_doc2md`: Conversion tools for document-to-Markdown flows, including DOCX, PPTX, XLSX, and math/OMML handling.
- `_utils`: Logging, CLI, and deprecation helpers.
- `_version.py`, `_constants.py`, `_env.py`, `_abstract.py`: Package support modules.

## Core OCR Internals: `ppocr`

The `ppocr` directory contains the lower-level OCR framework code used for model training and task-specific processing.

Main areas:

- `data`: Dataset and data loading components.
- `modeling`: Neural network architecture components.
- `losses`: Training loss implementations.
- `metrics`: Evaluation metrics for detection, recognition, table recognition, KIE, super-resolution, and end-to-end OCR.
- `optimizer`: Optimizer and learning-rate support.
- `postprocess`: Output decoding and task-specific post-processing.
- `utils`: General utilities, dictionaries, visualization, save/load helpers, logging, profiling, formula utilities, and end-to-end OCR helpers.
- `ext_op`: Custom/extension operation support.

## Document Structure Tools: `ppstructure`

The `ppstructure` directory contains document layout and structured-output tools.

Notable areas:

- `table`: Table detection/structure recognition, matching, evaluation, and Excel-like output helpers.
- `layout`: Layout prediction utilities.
- `kie`: Key information extraction tools and docs.
- `pdf2word`: PDF-to-Word tooling and GUI-related icons.
- `recovery`: Export/recovery helpers for reconstructing documents into Markdown or DOCX-like formats.
- Top-level prediction helpers such as `predict_system.py` and `utility.py`.

## Tools

The `tools` directory contains task scripts for working with models:

- Training: `train.py`, `program.py`
- Evaluation: `eval.py`
- Model export: `export_model.py`, `export_center.py`
- Inference scripts: detection, recognition, classification, end-to-end OCR, KIE, table, and super-resolution
- Runtime prediction helpers under `tools/infer`
- End-to-end OCR label conversion and evaluation helpers under `tools/end2end`

## Deployment Material

The `deploy` directory contains multiple deployment targets and examples:

- C++ inference assets and build scripts.
- Android demo project with Java, C++, Gradle, resources, and sample images.
- Paddle Lite / ARM / AVH examples.
- Serving and hubserving examples.
- Docker and high-performance service-oriented deployment assets.
- Slim/compression workflows such as quantization, pruning, and auto-compression.
- PaddleOCR-VL Docker/service configuration.

## Documentation And Assets

There are two major documentation areas:

- `docs`: Large versioned documentation tree with guides, algorithm notes, deployment instructions, images, and examples.
- `doc`: Font assets for rendering or visualization across many scripts and languages.

The `readme` directory contains localized README files for Chinese, Traditional Chinese, Japanese, Korean, French, Russian, Spanish, and Arabic.

## JavaScript And Integrations

The folder includes separate integration-oriented subprojects:

- `paddleocr-js`: Browser inference SDK code using TypeScript/JavaScript assets.
- `langchain-paddleocr`: LangChain document loader integration with unit/integration tests and sample files.
- `mcp_server`: MCP server package exposing PaddleOCR-related pipelines.

## Tests And Validation

Testing material appears in two main places:

- `tests`: Python tests for safe config/model loading, post-processing, formulas, PP-Structure, French accents, augmentation, and sample image/document files.
- `test_tipc`: Larger test suite for training/inference/deployment validation across Python, C++, serving, Lite, Paddle2ONNX, PTQ, benchmark, and web checks.

## Practical Takeaway

This folder is not a small OCR sample. It is a complete PaddleOCR source repository snapshot with:

- A modern installable `paddleocr` Python package.
- Legacy/core OCR internals under `ppocr`.
- Structured document processing under `ppstructure`.
- Training, evaluation, export, and inference scripts.
- Extensive documentation and multilingual README files.
- Deployment examples for Python, C++, Android, browser, Docker/service, and hardware-specific paths.
- Tests, benchmarks, sample files, and validation scripts.

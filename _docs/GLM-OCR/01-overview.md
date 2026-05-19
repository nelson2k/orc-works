# GLM-OCR Overview

## What This Folder Contains

`GLM-OCR` is an OCR and document understanding project built around a multimodal GLM OCR model. The local folder contains source code for a Python SDK, a CLI, cloud API integration, self-hosted OCR pipeline code, a backend task service, a browser frontend, examples, deployment notes, fine-tuning files, resources, and agent skill definitions.

The README describes the model as a multimodal OCR model for complex document understanding. It focuses on structured document parsing, layout-aware recognition, tables, formulas, seals, handwritten content, code-heavy pages, and real-world document layouts.

## Main Capabilities Present In The Folder

- Parse images and PDFs.
- Use a hosted MaaS API with only an API key.
- Use a self-hosted VLM endpoint through OpenAI-compatible APIs, SGLang/vLLM-style servers, Ollama generate mode, or MLX-style endpoints.
- Detect document layout regions through PP-DocLayoutV3.
- OCR layout regions in parallel.
- Format output as JSON and Markdown.
- Save layout visualizations and cropped image regions.
- Run an async backend task system for uploads and polling.
- Use a React frontend to upload a file, preview it, highlight OCR regions, and inspect Markdown/JSON output.
- Provide agent skills for OCR, table OCR, formula OCR, handwriting OCR, and SDK setup.

## Root Package Metadata

The root `pyproject.toml` defines the SDK package:

- Project name: `glmocr`
- Version: `0.1.5`
- Python requirement: `>=3.10`
- License: Apache-2.0
- Console script: `glmocr = glmocr.cli:main`
- Main package data: `glmocr/config.yaml`

Core dependencies:

- `pillow`
- `numpy`
- `requests`
- `pydantic`
- `PyYAML`
- `portalocker`
- `python-dotenv`
- `tqdm`
- `pymupdf`

Optional dependency groups:

- `layout`: OpenCV, Torch, TorchVision, Transformers, SentencePiece, Accelerate.
- `server`: Flask.
- `selfhosted`: layout dependencies plus `pypdfium2`.
- `all`: full SDK feature set.
- `dev`: pytest, black, flake8, mypy, pre-commit.

## Root Files

- `README.md`: English project overview, usage, SDK, and deployment information.
- `README_zh.md`: Chinese project overview.
- `pyproject.toml`: SDK package metadata, dependencies, optional extras, CLI entry point, and test/tool configuration.
- `LICENSE`: Apache-2.0 license.
- `agent.md`: Agent-facing usage/context document.
- `.gitignore`: Ignore rules.
- `.pre-commit-config.yaml`: Pre-commit configuration.

## Top-Level Directories

- `.github`: repository automation/configuration.
- `apps`: backend service, frontend app, Docker/start scripts.
- `examples`: input samples, output samples, deployment guides, fine-tuning material.
- `glmocr`: main Python SDK source.
- `resources`: visual and community assets.
- `skills`: agent skill definitions and helper scripts.

## Practical Reading

This source tree has two overlapping product surfaces:

- The SDK/CLI surface under `glmocr`, intended for direct Python or terminal use.
- The app surface under `apps`, intended for upload-based OCR task processing through an API and browser UI.

The example and skills folders sit beside those surfaces and demonstrate or package common usage patterns.

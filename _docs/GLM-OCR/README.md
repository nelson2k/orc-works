# GLM-OCR Documentation

This folder documents the local `GLM-OCR` source tree in depth. The notes are self-contained and describe what is inside that folder, how the pieces relate, and what each major subsystem does.

## Documents

- [01-overview.md](01-overview.md): project purpose, folder shape, package metadata, and major capabilities.
- [02-architecture.md](02-architecture.md): runtime architecture, MaaS mode, self-hosted mode, pipeline stages, threading model, and result flow.
- [03-sdk-and-cli.md](03-sdk-and-cli.md): Python SDK entry points, CLI behavior, configuration layering, clients, inputs, outputs, and save format.
- [04-core-pipeline.md](04-core-pipeline.md): page loading, layout detection, OCR request handling, post-processing, result formatting, and memory/queue tracking.
- [05-backend-service.md](05-backend-service.md): FastAPI backend, task model, worker lifecycle, pipeline flow, endpoints, database usage, and output generation.
- [06-frontend-app.md](06-frontend-app.md): React/Vite frontend structure, upload/poll flow, PDF/image preview, OCR block state, Markdown/JSON views, and interaction model.
- [07-examples-deployment-finetune.md](07-examples-deployment-finetune.md): sample files, generated results, self-hosting, multi-GPU, Ollama, MLX, and fine-tuning material.
- [08-skills-and-agent-assets.md](08-skills-and-agent-assets.md): skill folders, helper scripts, output schema reference, and agent-oriented workflows.
- [09-file-map.md](09-file-map.md): detailed local file and directory map with file counts and practical interpretation.

## Short Summary

`GLM-OCR` is a complete OCR/document-understanding project snapshot. It includes:

- A Python package named `glmocr`.
- A CLI command named `glmocr`.
- A hosted API/MaaS client path.
- A self-hosted document OCR pipeline with layout detection and parallel recognition.
- A FastAPI task backend.
- A React/Vite frontend for upload, preview, and result inspection.
- Example inputs, example outputs, deployment guides, fine-tuning material, visual resources, and agent skills.

The source is not a tiny demo. It is a multi-surface project: SDK, service, UI, examples, and skills all live in the same folder.

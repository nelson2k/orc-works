# Examples, Deployment, And Fine-Tuning

## Examples Folder

The `examples` directory contains sample inputs, sample results, deployment guides, and fine-tuning material.

Important areas:

- `example.py`: basic Python example.
- `source`: source files used as OCR examples.
- `result`: generated output samples.
- `multi-gpu-deploy`: multi-GPU deployment example.
- `self-host`: self-hosting guide.
- `ollama-deploy`: Ollama deployment guide.
- `mlx-deploy`: MLX deployment guide.
- `finetune`: fine-tuning guide, configs, and sample data.

## Source Samples

`examples/source` contains:

- `table.png`
- `seal.png`
- `paper.png`
- `page.png`
- `handwritten.png`
- `code.png`
- `GLM-4.5V.pdf`

These cover table recognition, seal/document examples, handwritten content, code-heavy content, normal page OCR, paper/document parsing, and multi-page PDF handling.

## Result Samples

`examples/result` contains generated outputs for the source examples.

For many samples, there are:

- Markdown output: `.md`
- JSON output: `.json`
- layout visualization images under `layout_vis`

For the `GLM-4.5V` PDF example, there are many page-level visualization images and cropped image outputs under `imgs`.

The result folder is large because it includes many rendered/cropped JPG outputs.

## Multi-GPU Deployment Example

`examples/multi-gpu-deploy` contains:

- `README.md`
- `README_zh.md`
- `launch.py`
- `coordinator.py`
- `worker.py`
- `engine.py`
- `gpu_utils.py`

This looks like a distributed/multi-GPU serving example with a coordinator/worker/engine split and launch helper.

## Self-Hosted Deployment

`examples/self-host/README.md` documents self-hosting.

The root README and config indicate self-hosted mode expects an OCR API service. Supported or discussed serving approaches include:

- vLLM
- SGLang
- Ollama
- MLX
- SDK server/client mode

Self-hosted SDK mode still uses local layout detection with PP-DocLayoutV3 and sends cropped regions to the configured VLM/OCR endpoint.

## Ollama Deployment

`examples/ollama-deploy/README.md` documents Ollama usage.

The SDK supports Ollama through `ocr_api.api_mode = ollama_generate`.

When in Ollama generate mode, `OCRClient` converts OpenAI-style chat requests into:

- `model`
- `prompt`
- `images`
- `stream=false`
- `options`

It maps generation parameters:

- `max_tokens` to `num_predict`
- `temperature` to `temperature`
- `top_p` to `top_p`
- `top_k` to `top_k`
- `repetition_penalty` to `repeat_penalty`

## MLX Deployment

`examples/mlx-deploy/README.md` documents Apple Silicon / MLX deployment.

The SDK supports model names and OpenAI-style endpoints, which lets a compatible MLX server be used as the OCR API.

## Fine-Tuning

`examples/finetune` contains:

- `README.md`
- `README_zh.md`
- `glm_ocr_lora_sft.yaml`
- `glm_ocr_full_sft.yaml`
- `cosyn_chemical.json`
- sample images under `cosyn_chemical`

This folder appears to document fine-tuning through LoRA or full supervised fine-tuning, with a small chemical document/image dataset sample.

## Resource Assets

The `resources` directory contains:

- `logo.svg`
- `docparse.png`
- `realworld.png`
- `speed.png`
- `wechat.jpg`
- `WECHAT.md`
- `PingFang.ttf`

These are presentation/community assets and a font file.

## Root App Start Scripts

The `apps` directory contains:

- `start-local.sh`
- `start-docker.sh`

These are startup helpers for running the app locally or with Docker.

The frontend and backend also each contain Docker-related files.

## What Is Not In The Folder

The folder contains SDK, app, examples, and deployment scaffolding, but the large GLM-OCR model weights are not stored as obvious root model files. The code expects models to be pulled from configured model IDs or served externally.

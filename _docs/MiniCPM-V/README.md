# MiniCPM-V

Small notes from the local `repos-folder/MiniCPM-V` repository.

MiniCPM-V is an OpenBMB multimodal model-family repository. It covers
vision-language models in the MiniCPM-V line and omnimodal models in the
MiniCPM-o line. The repo is not a single package; it is a collection of
model docs, inference examples, web demos, finetuning scripts, evaluation
harnesses, legacy model code, and media assets.

The main workflows are:

- run local image/video chat demos with Hugging Face `transformers`
- call hosted API endpoints for newer public models
- finetune supported models with `transformers`, DeepSpeed, and PEFT LoRA
- evaluate MiniCPM variants through VLM Eval Kit or VQA-style scripts
- run real-time MiniCPM-o 2.6 demos with FastAPI, WebSocket/SSE, and a Vue UI

The repository has both English and Chinese documentation. The root
`README.md` is very large and acts as the project homepage, model card,
usage guide, model zoo, and news log.

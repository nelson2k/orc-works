# Files in the snapshot

`du -sh repos-folder/Qwen2.5-VL-3B-Instruct-AWQ` → ~3.4 GB total.

| File | Size | What it's for |
|---|---|---|
| `model.safetensors` | 3.4 GB | All model weights — AWQ-4bit decoder + bf16 ViT + embeddings + scales/zeros. Single shard. |
| `tokenizer.json` | 10.9 MB | Fast tokenizer (HF `tokenizers` Rust serialization). |
| `vocab.json` | 2.6 MB | Byte-level BPE vocab (id → token string). |
| `merges.txt` | 1.7 MB | BPE merge rules. |
| `tokenizer_config.json` | 5.9 KB | Tokenizer init args + the chat template. |
| `chat_template.json` | 1.0 KB | The same Jinja chat template, separate file for processor classes that load it independently. |
| `added_tokens.json` | 0.6 KB | Map of special-token strings → ids (151 643 … 151 664). |
| `special_tokens_map.json` | 0.6 KB | Which of the special tokens map to BOS / EOS / PAD / additional. |
| `preprocessor_config.json` | 0.6 KB | `Qwen2VLImageProcessor` config — patch size, normalization, pixel bounds. See [image-pipeline.md](image-pipeline.md). |
| `config.json` | 1.5 KB | Model `Qwen2_5_VLConfig` + nested `vision_config` + AWQ `quantization_config`. See [architecture.md](architecture.md). |
| `generation_config.json` | 0.3 KB | Default sampling params used by `generate()` if not overridden. See [tokenizer-and-templates.md](tokenizer-and-templates.md#generation-defaults). |
| `README.md` | 13.4 KB | Upstream Hugging Face model card. |
| `LICENSE` | 7.3 KB | Qwen Research License. |
| `.gitattributes` | 1.6 KB | LFS filters for the model and tokenizer files. |

## What's NOT here

No Python module code — unlike e.g. `moondream2`, Qwen2.5-VL uses the **built-in** `Qwen2_5_VLForConditionalGeneration` class shipped with `transformers ≥ 4.49.0.dev0`. There's no `trust_remote_code=True`, no `modeling_*.py`, no `configuration_*.py` in the snapshot. Updating the snapshot can never change runtime code paths — only weights and config.

No fine-tuning artifacts (no optimizer state, no LoRA adapters, no training logs).

No example scripts — see the upstream README in `repos-folder/Qwen2.5-VL-3B-Instruct-AWQ/README.md` and [usage.md](usage.md) for examples.

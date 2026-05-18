# Model conversion

llama.cpp only loads GGUF files at runtime. Any model from Hugging Face, PyTorch, or another framework has to be converted first. The converters are Python scripts at the repo root (plus a few in `examples/` for legacy formats).

All of them depend on the `gguf` Python package shipped in this same repo (`gguf-py/`); the requirements files in `requirements/` pin the exact dependency set per script.

## convert_hf_to_gguf.py — the main converter

Run from the repo root:

```bash
python convert_hf_to_gguf.py \
    /path/to/huggingface/checkpoint \
    --outfile model.gguf \
    --outtype f16
```

Accepts a directory that looks like a standard HF checkpoint: `config.json`, one of `model*.safetensors` / `pytorch_model*.bin`, plus a tokenizer (`tokenizer.json` / `tokenizer.model` / `vocab.json` + `merges.txt` depending on the family).

Default behavior:

- Sniffs `config.json` `architectures` to pick a converter class.
- Loads tensors lazily via `numpy.memmap` so RAM usage stays low.
- Maps HF tensor names to canonical llama.cpp names using `gguf-py/gguf/tensor_mapping.py`.
- Loads the tokenizer; for BPE-based tokenizers it computes a *pre-tokenizer hash* (`tokenizer.ggml.pre`) so llama.cpp can pick the exact byte-level BPE behavior at inference.
- Fills the `general.*` and `<arch>.*` metadata.
- Writes a single output GGUF (or a sharded one with `--split-max-size`).

Useful flags:

| Flag | Effect |
|---|---|
| `--outfile FILE` | Output GGUF path. |
| `--outtype f32/f16/bf16/q8_0/auto` | Element type for tensors. `auto` follows source precision. |
| `--vocab-only` | Write only the metadata + tokenizer (used for testing). |
| `--bigendian` | Big-endian output (for IBM Z and similar). |
| `--split-max-size SIZE` | Shard output, e.g. `2G`. |
| `--split-max-tensors N` | Shard by tensor count. |
| `--no-tensor-first-split` | Don't ensure the first split contains all metadata. |
| `--metadata FILE` | Override `general.*` keys from a JSON file. |
| `--use-temp-file` | Stage intermediate output to disk (lower RAM). |
| `--model-name NAME` | Override `general.name`. |
| `--mmproj` | Emit only the multimodal projector (vision/audio sub-network) instead of the LLM. |
| `--print-supported-models` | List every architecture the script supports. |

Architectures supported are exactly the set that llama.cpp's C++ loader can handle: LLaMA 1/2/3, Mistral / Mixtral, Qwen 1.5 / 2 / 2.5 / 3, Phi 2/3/4 (and MoE), Gemma 1/2/3, GLM 4 / Edge, DeepSeek v1/v2/v3, Granite (text + MoE + hybrid), Hunyuan, Falcon, Baichuan, Aquila, InternLM2/3, Yi, OLMo / OLMo2 / OLMoE, MPT, Bloom, BERT, GPT-2, GPT-NeoX / Pythia, RWKV 6/7, Mamba, FalconMamba, Bitnet b1.58, Cohere Command-R, Snowflake-Arctic, DBRX, Jamba, LFM2, EXAONE, ChatGLM, SmolLM, Jais, Bielik, and many fine-tunes of all of the above.

## convert_lora_to_gguf.py — LoRA adapters

```bash
python convert_lora_to_gguf.py \
    --base /path/to/base-hf-checkpoint \
    /path/to/peft-lora-checkpoint \
    --outfile lora.gguf
```

Produces a GGUF LoRA adapter. Apply at inference time with `llama-cli ... --lora lora.gguf` or `--lora-scaled lora.gguf:0.5`.

`tools/export-lora/` can merge a LoRA GGUF into the base model to produce a new merged GGUF (so you don't pay the LoRA overhead per request).

## convert_llama_ggml_to_gguf.py — legacy GGML

Older GGML v1/v2/v3 files (pre-GGUF, from ~2023) can be upconverted:

```bash
python convert_llama_ggml_to_gguf.py --input old.ggml --output new.gguf
```

Detects the legacy magic and rewrites in GGUF v3 layout. Loses no quantized data — the existing weights are repacked into a GGUF wrapper.

## examples/convert_legacy_llama.py — pre-HF LLaMA

For the original Meta LLaMA-1 / LLaMA-2 checkpoints (`tokenizer.model` + `consolidated.NN.pth`):

```bash
python examples/convert_legacy_llama.py /path/to/llama-2-7b
```

Effectively unused now — every distribution since LLaMA-3 ships in HF format.

## examples/convert-llama2c-to-ggml — Karpathy's llama2.c

A small C program that reads a `model.bin` produced by Karpathy's [llama2.c](https://github.com/karpathy/llama2.c) and writes a GGUF.

```bash
# Build first as part of examples
./examples/convert-llama2c-to-ggml/llama2c-to-ggml --copy-vocab-from-model llama2.gguf \
    --llama2c-model stories15M.bin --llama2c-output-model stories15M.gguf
```

## convert_hf_to_gguf_update.py

Maintainer-side script. Regenerates the BPE pre-tokenizer hash table. Run when a new model family with a new byte-level BPE variant is added so that `convert_hf_to_gguf.py` can emit the right `tokenizer.ggml.pre` value.

## Multimodal projectors

For vision/audio models, `convert_hf_to_gguf.py --mmproj` produces a separate `mmproj-*.gguf` containing the vision encoder + projector head. Supported models include Gemma 3, SmolVLM, SmolVLM2, Pixtral 12B, Qwen 2 / 2.5 VL, Mistral Small 3.1, InternVL 2.5 / 3, MiniCPM-V 4.6, LFM2-VL.

For older multimodal models, conversion scripts live under `tools/mtmd/legacy-models/`.

## Auxiliary helpers in examples/

| Script | Purpose |
|---|---|
| `examples/json_schema_to_grammar.py` | Pure Python version of the JSON-schema → GBNF converter. Useful to inspect grammars without running C++. |
| `examples/pydantic_models_to_grammar.py` | Generate GBNF from a Pydantic model definition. |
| `examples/regex_to_grammar.py` | Compile a regex into GBNF. |
| `examples/ts-type-to-grammar.sh` | TypeScript-type → GBNF. |
| `examples/server_embd.py` | Demonstrate the embeddings route of llama-server. |

## Requirements

The `requirements/` directory has per-task pinned dependency sets:

| File | Use case |
|---|---|
| `requirements-all.txt` | Everything at once. |
| `requirements-convert_hf_to_gguf.txt` | Just the main converter. |
| `requirements-convert_hf_to_gguf_update.txt` | The maintainer update script. |
| `requirements-convert_legacy_llama.txt` | Legacy pre-HF LLaMA. |
| `requirements-convert_llama_ggml_to_gguf.txt` | Legacy GGML → GGUF. |
| `requirements-convert_lora_to_gguf.txt` | LoRA conversion. |
| `requirements-pydantic.txt` | The pydantic-grammar example. |
| `requirements-gguf_editor_gui.txt` | The Qt GUI editor. |
| `requirements-compare-llama-bench.txt` | Bench result comparator. |
| `requirements-server-bench.txt` | Load-testing the server. |
| `requirements-test-tokenizer-random.txt` | Tokenizer fuzz tests. |
| `requirements-tool_bench.txt` | Function-calling bench. |

The top-level `requirements.txt` is a meta-file that pulls everything via `-r`. `pyproject.toml` is for the `gguf-py` sub-package only — the converters at the repo root are scripts, not an installable package.

## End-to-end recipe

```bash
# Get the HF checkpoint
huggingface-cli download mistralai/Mistral-7B-Instruct-v0.3 --local-dir mistral-7b

# Convert to F16 GGUF
python convert_hf_to_gguf.py mistral-7b --outfile mistral-7b-f16.gguf

# (Optional) Build an importance matrix for IQ quants
llama-imatrix -m mistral-7b-f16.gguf -f wiki.train.raw -o imatrix.gguf

# Quantize to Q4_K_M
llama-quantize --imatrix imatrix.gguf mistral-7b-f16.gguf mistral-7b-q4km.gguf Q4_K_M

# Run
llama-cli -m mistral-7b-q4km.gguf -p "Hello"
```

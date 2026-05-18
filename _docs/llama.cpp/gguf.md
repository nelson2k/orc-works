# GGUF — the model file format

GGUF (GGML Universal File) is the sole model file format llama.cpp loads. It is a single binary file with a header, a flat key/value metadata map, a list of tensor infos, and a tensor data section. Everything needed to load and run a model — architecture identifier, hparams, tokenizer, weights — lives in the file.

## Format at a glance

```
struct gguf_file {
    uint32_t  magic        = "GGUF";
    uint32_t  version      = 3;
    uint64_t  tensor_count;
    uint64_t  metadata_kv_count;

    // metadata KV
    gguf_kv   metadata[metadata_kv_count];

    // tensor infos
    gguf_tensor_info  tensors[tensor_count];

    // [padding to alignment]

    // tensor data (raw bytes per tensor)
    uint8_t   data[];
}
```

Each `gguf_kv` is `(string name, gguf_type type, value)`. `gguf_type` covers `UINT8/16/32/64`, `INT8/16/32/64`, `FLOAT32/64`, `BOOL`, `STRING`, `ARRAY` (which itself carries an inner type and length).

Each `gguf_tensor_info` is `(string name, uint32 n_dims, uint64 dims[n_dims], ggml_type, uint64 offset_into_data)`. Weights are stored in `ggml_type` element types — F32, F16, BF16, Q4_0, …, Q4_K, …, MXFP4, etc.

## Conventional metadata keys

GGUF is schema-free, but llama.cpp expects (and writes) a conventional set of keys:

| Key | Meaning |
|---|---|
| `general.architecture` | "llama" / "qwen3" / "gemma3" / "mistral" / "phi3" / "rwkv6" / … — chooses the C++ model class |
| `general.name` | Human-readable display name |
| `general.quantization_version` | Quant-format version |
| `general.file_type` | Quant scheme used for tensor data |
| `<arch>.context_length` | Max sequence length |
| `<arch>.embedding_length` | Hidden dim |
| `<arch>.block_count` | Number of transformer layers |
| `<arch>.feed_forward_length` | FFN inner dim |
| `<arch>.attention.head_count`, `.head_count_kv` | Attention head counts |
| `<arch>.attention.layer_norm_rms_epsilon` | RMSNorm eps |
| `<arch>.rope.dimension_count`, `.freq_base`, `.scaling.type` | RoPE config |
| `tokenizer.ggml.model` | "llama" / "gpt2" / "bert" / "t5" / "rwkv" / "plamo2" |
| `tokenizer.ggml.pre` | Pre-tokenizer hash (BPE byte-level variant) |
| `tokenizer.ggml.tokens` | String array of vocabulary |
| `tokenizer.ggml.scores`, `.token_type`, `.merges` | Per-token data |
| `tokenizer.ggml.bos_token_id`, `eos_token_id`, `pad_token_id`, `unknown_token_id` | Special-token ids |
| `tokenizer.chat_template` | Jinja chat template (single or per-template-name) |

For multimodal models there's an additional `clip.*` namespace, and for vision projectors there's `general.architecture = "clip"` plus projector-specific keys.

## Splits

A logical model can be sharded across files. The naming convention is:

```
my-model-00001-of-00005.gguf
my-model-00002-of-00005.gguf
...
my-model-00005-of-00005.gguf
```

The libllama helpers `llama_split_path(prefix, n, count)` and `llama_split_prefix(path, n, count)` round-trip these. `tools/gguf-split` shards a single GGUF into N parts or merges them back.

## Two implementations

### C++ (in this repo)

`ggml/include/gguf.h` defines `gguf_context` and the read/write API. Used by libllama, every tool, and the `tools/quantize` binary. Streaming-friendly: tensor data is referenced by offset, not loaded eagerly.

### Python: `gguf-py/gguf/`

A standalone Python package published to PyPI as `gguf`. Used by all `convert_*.py` scripts and by anyone who wants to read or write GGUFs from Python.

Layout:

- `constants.py` — all the key strings, GGUF types, file-type enums, ggml-type sizes.
- `gguf_reader.py` — read a file lazily (mmap), iterate metadata + tensors.
- `gguf_writer.py` — incremental writer: open, add metadata, add tensor headers, then write data.
- `lazy.py` — lazy tensor wrappers backed by a `numpy.memmap`.
- `metadata.py` — heuristics for filling `general.*` from HF model cards.
- `tensor_mapping.py` — name remapping from common upstream conventions (LLaMA / HF / Transformers / GPT-NeoX / ...) into the canonical llama.cpp tensor names.
- `vocab.py` — readers for SentencePiece, HF tokenizer.json, BPE merges.
- `quants.py` — Python implementations of the q-formats (used for reference and verification, not for fast quantization).
- `scripts/` — CLI entry points exposed as console scripts:
  - `gguf-dump` — list metadata and tensor info.
  - `gguf-set-metadata` — set a single key.
  - `gguf-new-metadata` — copy a file with metadata changes (add / remove / modify keys).
  - `gguf-convert-endian` — flip byte order.
  - `gguf-editor-gui` — Qt-based viewer/editor (`pip install gguf[gui]`).

## Tools that operate on GGUF

| Tool | Use |
|---|---|
| `llama-quantize` | Re-quantize a GGUF to a different element type (per-tensor) |
| `llama-imatrix` | Generate an importance matrix for higher-quality quantization |
| `gguf-split` | Shard / merge GGUF files |
| `gguf-dump` (Python) | Inspect metadata + tensor list |
| `gguf-new-metadata` (Python) | Patch metadata without rewriting tensor data |
| `gguf-editor-gui` (Python) | GUI inspector |
| `llama-tokenize` | Round-trip text ↔ tokens against a model's vocab |

## How a GGUF is born

The standard path:

1. Download HF checkpoint (`safetensors` + `tokenizer.json` + `config.json`).
2. Run `python convert_hf_to_gguf.py /path/to/checkpoint` → produces a F16 / BF16 / F32 GGUF.
3. Optionally run `llama-quantize in.gguf out.gguf Q4_K_M` (or any other quant) to shrink it.
4. Optionally run `llama-imatrix -m in.gguf -f calibration.txt -o imatrix.gguf` first, then pass `--imatrix imatrix.gguf` to `llama-quantize` for better IQ-quant accuracy.

For legacy formats:

- `convert_llama_ggml_to_gguf.py` — old GGML v1/v2/v3 → GGUF.
- `convert_lora_to_gguf.py` — HF LoRA adapter → GGUF LoRA.
- `examples/convert_legacy_llama.py` — pre-HF LLaMA checkpoint (`tokenizer.model` + `consolidated.NN.pth`) → GGUF.
- `examples/convert-llama2c-to-ggml/` — Karpathy's llama2.c checkpoints.

## Versions

The current GGUF version is **3**. Older formats are still readable. The format itself is stable across llama.cpp versions; the *metadata schema* evolves (new keys may be added when a new architecture is supported), but unknown keys are ignored on load.

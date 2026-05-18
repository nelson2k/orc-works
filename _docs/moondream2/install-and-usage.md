# Install and usage

The model is loaded as a Hugging Face checkpoint with `trust_remote_code=True`. There is no Python package to `pip install` for the model itself — `transformers` does the right thing if a few extras are present.

## Dependencies

| Package | Why |
|---|---|
| `transformers` ≥ 4.44 | HF loader, generation utilities. The shipped config was generated with `transformers 4.52.4`. |
| `torch` (with CUDA / MPS) | Tensor backend. bf16 inference; CUDA, Apple MPS, and CPU all supported. |
| `safetensors` | Reading `model.safetensors`. Comes with `transformers`. |
| `tokenizers` | The `tokenizers.Tokenizer` is used directly (not the HF auto-tokenizer wrapper) for the in-repo BPE. |
| `Pillow` | PIL image input. |
| `einops` | Used by the vision code (listed in `requirements.txt`). |
| `pyvips` + `pyvips-binary` | Optional but recommended. `image_crops.py` uses libvips when available for fast image slicing; falls back to PIL otherwise. |
| `torchao` | **Optional**. Only needed if you load the int4-quantized variant — the `QuantizedLinear.unpack()` path imports `torchao.quantize_` and `int4_weight_only`. Without torchao, plain bf16 / fp16 loads still work. |

```bash
pip install transformers torch pillow einops pyvips pyvips-binary
# Optional, for int4 quant
pip install torchao
```

`pyvips-binary` ships the libvips DLL bundle on Windows. On Linux/macOS you may need a system `libvips` install (`apt install libvips`, `brew install vips`).

## Basic usage

```python
from transformers import AutoModelForCausalLM
from PIL import Image

model = AutoModelForCausalLM.from_pretrained(
    "vikhyatk/moondream2",
    revision="2025-06-21",
    trust_remote_code=True,
    device_map={"": "cuda"},   # or "mps", or "cpu"
)

img = Image.open("photo.jpg")

# Short caption
print(model.caption(img, length="short")["caption"])

# VQA
print(model.query(img, "How many people are in this image?")["answer"])

# Streamed response
for chunk in model.query(img, "Describe the scene.", stream=True)["answer"]:
    print(chunk, end="", flush=True)

# Object detection
for box in model.detect(img, "face")["objects"]:
    print(box)
```

## Pinning a revision

The model is updated frequently and breaking changes happen (new template ids, tokenizer swaps, new region-head shapes). **Always pin** in production:

```python
AutoModelForCausalLM.from_pretrained(
    "vikhyatk/moondream2",
    revision="2025-06-21",        # ← pick a date from versions.txt
    trust_remote_code=True,
    ...
)
```

Available revisions (from `versions.txt`):

```
2024-03-04, 2024-03-06, 2024-03-13, 2024-04-02, 2024-05-08, 2024-05-20,
2024-07-23, 2024-08-26, 2025-01-09, 2025-03-27, 2025-04-14, 2025-06-21
```

## Reusing an `EncodedImage`

Vision encoding is the most expensive single step. If you plan to run multiple skills against the same image, call `encode_image` once and reuse the result:

```python
enc = model.encode_image(img)
print(model.caption(enc, length="short")["caption"])
print(model.query(enc, "What's happening?")["answer"])
for box in model.detect(enc, "person")["objects"]:
    print(box)
```

Each skill accepts an `EncodedImage` anywhere a `PIL.Image` is expected — `encode_image` is a no-op when called with one already.

## Sampling control

```python
model.query(
    img,
    "...",
    settings={
        "max_tokens":  512,
        "temperature": 0.7,
        "top_p":       0.9,
        "variant":     "some-fine-tune-id",
    },
)

model.detect(img, "face", settings={"max_objects": 20})
```

Set `temperature=0` for deterministic / greedy output.

## Device choices

- **CUDA** (`device_map={"": "cuda"}`) — primary path. bf16 throughout. Multi-GPU `device_map="auto"` should work but most of the model is small enough to fit on one GPU.
- **MPS** (Apple Silicon) — works. The vision code has an MPS-specific shim around `adaptive_avg_pool2d` (non-divisible input sizes aren't implemented on MPS yet, so the pool runs on CPU and returns to MPS).
- **CPU** — works for testing; slow.

## `torch.compile`

```python
model.compile()        # accelerates _vis_enc / _prefill / _decode_one_tok
```

Calling `compile()` on `MoondreamModel` (the underlying model under `HfMoondream.model`) enables `torch.compile(fullgraph=True)` on the three hot paths, plus `mode="reduce-overhead"` for `_decode_one_tok`. `_vis_proj` is **not** currently compiled.

Side effects to be aware of:

- `compile()` calls `.unpack()` on every `QuantizedLinear` first — so if you loaded the int4 variant, all quantized linears get unpacked into plain bf16 linears and re-quantized via `torchao.quantize_(model, int4_weight_only(group_size=128))`. This is a one-time cost at compile time.
- `torch._dynamo.mark_dynamic` is used in a few spots; recompiles can happen when input shapes change in unsupported ways.

## Endpoint deployment

`handler.py` is the inference-endpoint entry point (HF Inference Endpoints / SageMaker / similar). Shape:

```python
from handler import EndpointHandler
h = EndpointHandler(model_dir)
response = h({"inputs": {"image": "<base64-encoded image bytes>", "question": "..."}})
```

It always uses `model.answer_question(enc_image, question, tokenizer)` (the legacy API), and returns:

```json
{"statusCode": 200, "body": {"answer": "..."}}
```

Errors come back as `{"statusCode": 500, "body": {"error": "..."}}`.

This handler does **not** expose `detect`, `point`, or `query(reasoning=True)` — it's a minimal VQA shim. If you need the full skill set in a deployment, wrap the model directly with FastAPI / Flask instead of using the supplied handler.

## Memory footprint

Rough numbers on a 24-layer / 2048-dim / 32-head text decoder + 27-layer / 1152-dim vision encoder + ~24 MB region head, all in bf16:

| Component | Approx. params |
|---|---|
| Vision encoder | ~430 M |
| Text decoder | ~1.4 B |
| Embedding (`wte`) | ~105 M (51 200 × 2048) |
| Region head | ~70 M |
| **Total** | **~2.0 B** |

At bf16 that's ~3.9 GB of weights. Add the KV cache buffer for the 2048-token context: `24 layers × 2 (K, V) × 32 heads × 2048 tokens × 64 head_dim × 2 bytes ≈ 800 MB`. Single inference reasonably fits in 6–8 GB of VRAM with comfortable headroom.

## Notes / caveats

- `tokenizer = Tokenizer.from_pretrained("moondream/starmie-v1")` runs at model init and needs network the first time (or an HF cache hit). Offline runs should pre-warm the HF cache.
- The first call to a skill triggers `_setup_caches()`, which allocates all the KV cache buffers on `model.device`. This adds a noticeable one-off cost.
- `device_map={"": "cuda"}` is the recommended single-GPU placement. Bare `.to("cuda")` after load also works but is slower (loads to CPU first).
- The `compile()` API is on the underlying `MoondreamModel`, accessed via `hf_model.model.compile()` from the HF wrapper.

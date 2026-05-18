# Multimodal (`libmtmd`)

Vision and audio inputs are handled by a sub-project, `libmtmd`, living in `tools/mtmd/`. It is a separate library from libllama because the pre-processing and projection steps differ wildly between vision models, and adding them all to the core would be unwieldy. Two GGUF files are loaded for a multimodal run:

1. The **text LLM** GGUF (the standard one).
2. A **multimodal projector** GGUF (`mmproj-*.gguf`) — contains the vision/audio encoder and the projector head that maps image/audio embeddings into the LLM's input embedding space.

The text model and the projector must come from the same family; you can't mix and match.

## What libmtmd does

For each image (or audio clip):

1. Pre-process per model (resize / patch-split / normalize for vision; mel-spec for audio).
2. Run the encoder (`clip.cpp` / `mtmd-image.cpp` / `mtmd-audio.cpp`) to get a sequence of vision/audio tokens.
3. Apply the projector to map to the LLM's `n_embd`.
4. Hand the resulting embeddings to libllama (`llama_batch.embd[]`) at the position the placeholder token would occupy in the prompt.
5. The LLM continues normally — text tokens before and after the multimodal block, multimodal embeddings in the middle, decode as usual.

This unification means **the same libmtmd CLI / API works for many model families** (LLaVA 1.5/1.6, Gemma 3, Qwen2-VL, Qwen2.5-VL, Mistral Small 3.1, InternVL 2.5/3, MiniCPM-V 4.6, MiniCPM-O 2.6/4.0, MobileVLM, GLM-Edge, Pixtral, SmolVLM, SmolVLM2, LFM2-VL, Moondream, granitevision, …).

## Files

- `mtmd.{h,cpp}` — public `libmtmd` API: load a projector, tokenize+encode a media item, produce embeddings.
- `mtmd-image.{h,cpp}` — image preprocessing + ViT encoder.
- `mtmd-audio.{h,cpp}` — audio preprocessing + audio encoder (mel-spec → encoder).
- `mtmd-helper.{h,cpp}` — small helpers that compose libmtmd with a libllama context.
- `mtmd-cli.cpp` — `mtmd-cli` binary: a unified CLI replacing the old per-model `llava-cli`, `qwen2vl-cli`, `minicpmv-cli`, `gemma3-cli` tools.
- `clip.{h,cpp}` / `clip-model.h` / `clip-graph.h` / `clip-impl.h` — the CLIP/ViT-style transformer that powers most vision encoders.
- `legacy-models/` — conversion scripts for old multimodal checkpoints whose projectors don't yet round-trip through `convert_hf_to_gguf.py --mmproj`.
- `models/` — small reference projectors for tests.
- `tests/`, `tests.sh`, `test-1.jpeg`, `test-2.mp3` — smoke tests.

## How to obtain an `mmproj`

For modern models, `convert_hf_to_gguf.py --mmproj` extracts the projector from the HF checkpoint:

```bash
python convert_hf_to_gguf.py /path/to/gemma-3-12b-it --mmproj --outfile mmproj.gguf
```

Supported via this path: Gemma 3 (≥ 4 B; the 1 B variant has no vision), SmolVLM / SmolVLM2, Pixtral 12 B, Qwen 2 VL / Qwen 2.5 VL, Mistral Small 3.1, InternVL 2.5 / 3, MiniCPM-V 4.6, LFM2-VL.

For older models the right path is one of the helper scripts in `tools/mtmd/legacy-models/` (LLaVA 1.5, LLaVA 1.6, MobileVLM, GLM-Edge, Moondream, MiniCPM-V 2.5 / 2.6, MiniCPM-O 2.6 / 4.0, granitevision, …). Each has its own README under `docs/multimodal/`.

Plenty of pre-built `mmproj` GGUFs are already published on Hugging Face — search for `<model> mmproj`.

## CLI usage

```bash
# Image
mtmd-cli -m text-model.gguf --mmproj mmproj.gguf --image cat.jpg -p "Describe this picture."

# Audio
mtmd-cli -m text-model.gguf --mmproj mmproj-audio.gguf --audio voice.mp3 -p "Transcribe."

# Pull both from HF in one command
mtmd-cli -hf openbmb/MiniCPM-V-4_6-GGUF --image input.png -p "What's here?"
```

`-hf user/repo` will pull the matching `mmproj-*.gguf` along with the model automatically. To skip the auto-mmproj fetch, add `--no-mmproj`.

## In `llama-server`

The same projector loads into `llama-server`:

```bash
llama-server -m text.gguf --mmproj mmproj.gguf -ngl 99
```

Image/audio inputs come in via OpenAI-style multimodal content arrays:

```json
{
  "role": "user",
  "content": [
    {"type": "text", "text": "What is in this image?"},
    {"type": "image_url", "image_url": {"url": "data:image/png;base64,..."}}
  ]
}
```

`audio_url` works analogously for audio. Server-side, libmtmd encodes the media once per request and the result feeds into the existing `/v1/chat/completions` slot.

## In C

```c
#include "mtmd.h"
struct mtmd_context * mctx = mtmd_init_from_file("mmproj.gguf", model, /*params*/ {0});
struct mtmd_bitmap * img = mtmd_bitmap_init_from_file("cat.jpg");
struct mtmd_input_chunks * chunks = mtmd_input_chunks_init();
mtmd_tokenize(mctx, chunks, /*text*/ "<image>describe", img, /*n_imgs*/ 1);

// Pass each chunk to llama_decode — text chunks as tokens, image chunks as embeddings.
mtmd_helper_eval_chunks(mctx, llama_ctx, chunks, ...);
```

The exact API is in `tools/mtmd/mtmd.h`.

## Stability

The mtmd subproject is under heavy churn — the README explicitly states **breaking changes are expected**. Pinning a llama.cpp version is wise if you build on top of `libmtmd`.

## Per-model guides

`docs/multimodal/` carries a short guide per family:

- `gemma3.md`
- `glmedge.md`
- `granitevision.md`
- `llava.md`
- `MobileVLM.md`
- `minicpmv2.5.md`, `minicpmv2.6.md`, `minicpmv4.0.md`, `minicpmv4.5.md`, `minicpmv4.6.md`
- `minicpmo2.6.md`, `minicpmo4.0.md`

Top-level `docs/multimodal.md` lists known pre-quantized model + projector pairs.

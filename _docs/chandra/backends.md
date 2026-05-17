# Backends — `hf` vs `vllm`

Picked with `--method` on the CLI or `method="hf"|"vllm"` to
`InferenceManager(method=...)`. Both produce identical `BatchOutputItem`s.

## `hf` (`chandra/model/hf.py`)

- Loads the model in-process with `transformers.AutoModelForImageTextToText`
  + `AutoProcessor` from `MODEL_CHECKPOINT`.
- `dtype=torch.bfloat16`, `device_map="auto"` (or `{"": TORCH_DEVICE}` if set).
- Optional `attn_implementation=TORCH_ATTN` — set to `"flash_attention_2"` if
  you've installed flash-attn.
- One `model.generate()` per batch. Default batch size is **1** on the CLI
  because each page is a single long-context generation.
- Stop tokens: both `<|endoftext|>` (from `generation_config`) and `<|im_end|>`
  (added explicitly since the model emits it at turn boundaries).
- **No retry / repeat-detection logic.** What the model emits is what you get.

Use this when: you want a self-contained single-process script, or you're
GPU-rich but don't want to run a separate server.

## `vllm` (`chandra/model/vllm.py`)

- Talks to a running vLLM server via the OpenAI-compatible chat API
  (`/v1/chat/completions`), images sent as base64-PNG data URLs.
- Launch the server separately — easiest is `chandra_vllm` (see [apps.md](apps.md)).
- Concurrency: `ThreadPoolExecutor(max_workers=min(64, len(batch)))`.
- **Repeat-token retries**: `detect_repeat_token` looks for a suffix that
  loops too many times; if it does, retry with bumped temperature
  (`temperature + 0.2*(retries+1)`, capped at 0.8) and `top_p=0.95`. Up to
  `MAX_VLLM_RETRIES=6` retries.
- Sleeps `2*(retries+1)` seconds on transient errors.
- `temperature=0.0`, `top_p=0.1` on the first attempt.

Use this when: production, batch jobs, or any time throughput matters. The
README cites **1.44 pages/s on a single H100** with 96 concurrent sequences.

## Quick decision matrix

| Concern                              | hf | vllm |
|--------------------------------------|----|------|
| Single GPU machine, one-off PDF      | ✓  |      |
| Throughput, many docs                |    | ✓    |
| Need automatic retries on hallucinations |    | ✓    |
| Want to avoid running a server       | ✓  |      |
| Multi-GPU / multi-node               |    | ✓    |
| Lightweight install (no torch)       |    | ✓    |

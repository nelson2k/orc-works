# Tools

Everything under `tools/` ships as a `llama-*` executable in the build directory. They all share the same argument parser (the `common/arg.cpp` library) and the same model-loading code path.

## llama-cli

`tools/cli/cli.cpp` — interactive or batch text generation. The reference "use the model from a shell" binary.

```bash
llama-cli -m model.gguf -p "Once upon a time"
llama-cli -hf ggml-org/gemma-3-1b-it-GGUF -p "..." -n 200
llama-cli -m model.gguf -i             # interactive REPL
llama-cli -m model.gguf -cnv           # conversation mode using model's chat template
llama-cli -m model.gguf -f prompt.txt -ngl all
```

Supports: prompt file, JSON-grammar response, stop sequences, antiprompt, sampler chain, KV cache type, LoRA, control vectors, multi-GPU split, function-calling, speculative decoding (draft model with `-md`), FIM completion (prefix/suffix tags), session save/load.

## llama-server

`tools/server/` — pure C/C++ HTTP server. Implements:

- **OpenAI-compatible**: `POST /v1/chat/completions`, `POST /v1/completions`, `POST /v1/embeddings`, `POST /v1/rerank`, `POST /v1/responses` (assistant-prefill).
- **Anthropic-compatible**: `POST /v1/messages`.
- Native endpoints: `GET /health`, `GET /metrics`, `GET /props`, `POST /tokenize`, `POST /detokenize`, `POST /infill`, `POST /apply-template`, `POST /slots`, `GET /props`.
- A built-in WebUI (`tools/ui/`, Svelte + Vite) bundled into the binary, served at `/`.

Features: continuous batching for many concurrent users, schema-constrained JSON, function calling for almost any model, multimodal, speculative decoding, parallel decoding. See server.md for details.

## llama-bench

`tools/llama-bench/` — performance harness.

```bash
llama-bench -m model.gguf                          # default suite
llama-bench -m model.gguf -p 0 -n 128              # generation only, 128 tokens
llama-bench -m model.gguf -p 512 -n 0              # prompt processing only
llama-bench -m model.gguf -ngl 0,10,20,99          # sweep gpu-layers
llama-bench -m a.gguf -m b.gguf -t 4,8,16          # multiple models / thread counts
llama-bench -m m.gguf -o json > out.json           # JSON output (also csv, jsonl, sql, md)
```

It runs each combination 5× by default (`-r 5`) and reports prompt-eval tok/s, generation tok/s, and total time per case. Output formats: markdown (default), CSV, JSON, JSONL, SQL.

## llama-quantize

`tools/quantize/` — quantize a F16/BF16/F32 GGUF to a smaller element type.

```bash
llama-quantize input-f16.gguf output-q4km.gguf Q4_K_M
llama-quantize --imatrix imatrix.gguf in.gguf out.gguf IQ3_S
llama-quantize --include-weights blk.0.attn_q.weight --leave-output-tensor in.gguf out.gguf Q5_K_M
```

Common quant targets: `Q4_K_M` (good default), `Q5_K_M` (more quality), `Q6_K`, `Q8_0`, `Q3_K_S` (smallest reasonable), and the IQ-family (`IQ2_XXS`, `IQ2_XS`, `IQ3_XXS`, `IQ3_S`, `IQ4_XS`, `IQ4_NL`, `IQ1_S`, `IQ1_M`) which need an importance-matrix file for best quality.

Useful flags:

- `--imatrix FILE` — use the importance matrix produced by `llama-imatrix`.
- `--leave-output-tensor` — keep the output projection in higher precision.
- `--pure` — don't auto-upgrade certain tensors (the default behavior promotes a few small but accuracy-sensitive ones).
- `--allow-requantize` — re-quantize an already-quantized model (lossy, generally avoid).
- `--include-weights` / `--exclude-weights` — per-tensor inclusion.
- `--output-tensor-type`, `--token-embedding-type` — override types for those specific tensors.

## llama-imatrix

`tools/imatrix/` — compute an importance matrix (per-tensor activation statistics) over a calibration dataset. Used to make IQ-quants much better than naive RTN.

```bash
llama-imatrix -m model-f16.gguf -f wiki.train.raw -o imatrix.gguf
llama-imatrix -m model-f16.gguf -f calib.txt --chunk 512 --process-output
```

Output is a small GGUF (or `.dat` if `--output-format dat`) that you feed to `llama-quantize --imatrix`.

## llama-perplexity

`tools/perplexity/` — evaluates perplexity on a text dataset. Other supported metrics:

- `--ppl` — perplexity.
- `--hellaswag` — HellaSwag accuracy.
- `--winogrande` — WinoGrande accuracy.
- `--multiple-choice` — generic MC accuracy.
- `--kl-divergence` — KL-divergence between two models (e.g. quantized vs. F16).

Useful for comparing quants quantitatively.

## llama-tokenize

`tools/tokenize/` — tokenizer debugger. Round-trips text through the model's tokenizer:

```bash
llama-tokenize -m model.gguf -p "Hello, world!"
llama-tokenize -m model.gguf -p "Hello" --no-bos --no-parse-special
```

Prints token ids and a decoded preview, useful when chasing tokenizer mismatches.

## llama-parser

`tools/parser/` — test the chat-template + auto-parser stack. Given a model and a candidate response, it shows how llama.cpp would extract tool calls / reasoning blocks.

## mtmd-cli

`tools/mtmd/` — multimodal CLI for image + audio input. Powered by `libmtmd`. Replaces the older per-model CLIs (`llava-cli`, `qwen2vl-cli`, `minicpmv-cli`, `gemma3-cli`).

```bash
mtmd-cli -m text-model.gguf --mmproj mmproj.gguf --image cat.jpg -p "Describe this image."
mtmd-cli -m text-model.gguf --mmproj mmproj-audio.gguf --audio voice.mp3 -p "Transcribe."
```

## llama-tts

`tools/tts/` — text-to-speech using OuteTTS-style models (a text LLM that generates audio tokens, plus a vocoder). Requires the vocoder GGUF via `--hf-repo-v / --hf-file-v`.

```bash
llama-tts -m outetts.gguf --hf-repo-v ggml-org/wavtokenizer --hf-file-v wavtokenizer-q8_0.gguf -p "Hello."
```

## rpc-server

`tools/rpc/` — exposes a ggml backend over TCP. Run it on a remote GPU box and point any tool's `--rpc <host>:<port>` at it; libllama will treat it as a local device.

```bash
# On the remote
rpc-server -p 50052 -H 0.0.0.0

# On the client
llama-cli -m model.gguf --rpc 192.168.1.42:50052 -ngl 99
```

## gguf-split

`tools/gguf-split/` — split a large GGUF into N parts (by max tensors per part or by size) and merge them back.

```bash
gguf-split --split --split-max-size 2G big.gguf big-part
gguf-split --merge big-part-00001-of-00005.gguf big-restored.gguf
```

Useful for repos like Hugging Face which had a 50 GB per-file limit historically.

## llama-batched-bench

`tools/batched-bench/` — sweeps the parallel-decoding throughput across various batch shapes (number of parallel sequences × prompt length × generation length).

## llama-cvector-generator

`tools/cvector-generator/` — train a control vector from a pair of contrastive text datasets.

## llama-export-lora

`tools/export-lora/` — merge a LoRA adapter back into the base GGUF, producing a new merged GGUF.

## llama-fit-params

`tools/fit-params/` — search the `--fit` parameter space to find the largest context / batch that fits on a given device's VRAM.

## llama-completion

`tools/completion/` — emits shell completion scripts for bash / zsh / fish / PowerShell.

## Shared argument set

Almost every tool inherits the same hundreds of flags from `common/arg.cpp`. Highlights:

- Loading: `-m`, `-hf user/repo[:quant]`, `-hff`, `-mu URL`, `-dr docker_repo`, `-hft TOKEN`, `--lora`, `--control-vector`, `--override-kv`, `--check-tensors`.
- Context: `-c`, `-b`, `-ub`, `-np`, `--keep`, `--swa-full`, `-fa`, `-ctk`, `-ctv`.
- Devices: `-ngl`, `-sm`, `-ts`, `-mg`, `-dev`, `--rpc`, `-ot`, `-cmoe`, `-ncmoe`, `-fit`, `-fitt`, `-fitc`.
- Sampling: `--temp`, `--top-k`, `--top-p`, `--min-p`, `--typical`, `--repeat-penalty`, `--mirostat`, `--samplers`, `--grammar`, `--grammar-file`, `--json-schema`, `--xtc-probability`.
- Logging: `--log-file`, `--log-disable`, `--log-colors`, `-v`, `-lv`, `--log-timestamps`.
- RoPE / YaRN: `--rope-scaling`, `--rope-scale`, `--rope-freq-base`, `--rope-freq-scale`, `--yarn-*`.

Almost every flag also has an `LLAMA_ARG_*` env-var equivalent for non-interactive use.

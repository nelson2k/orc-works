# Integrations

MinerU is positioned as the **document-ingestion layer for RAG / agent
stacks**. The repo + ecosystem expose explicit hooks for:

## MCP server (AI coding tools)

MinerU ships as an MCP (Model Context Protocol) server so Cursor / Claude
Desktop / Windsurf / any MCP client can drop a PDF and get markdown back
through a tool call.

Setup is documented at the MinerU docs site; install the MCP server
separately from `mineru-mcp-server` (lives outside this repo).

Usage pattern: the MCP server wraps `mineru-api` — your AI tool calls
something like `parse_pdf(path)` and gets structured markdown.

## RAG frameworks

The README lists native integrations / community connectors for:

- **LangChain** — `langchain-mineru` loader (third-party).
- **LlamaIndex** — `MinerULoader` reader.
- **RAGFlow** — built-in document parser option.
- **RAG-Anything** — Knowledge-graph-aware RAG; uses MinerU for parsing.
- **Flowise** — node available in low-code workflows.
- **Dify** — first-class document type.
- **FastGPT** — file parsing endpoint.

Each uses `mineru-api` under the hood (sync or async), so feature parity
follows the API. They mostly add nicer auth, chunking, and metadata
mapping.

## SDKs and clients

- Python: `pip install mineru` gets you the orchestration client.
  Programmatic use: import from `mineru.cli.client` (e.g.
  `collect_input_documents`, `run_orchestrated_cli`) — but mostly you
  should hit `mineru-api` directly via `httpx`.
- Go / TypeScript: official SDKs available outside this repo.
- REST: documented OpenAPI at `mineru-api`'s `/docs` (FastAPI auto-gen).
- Docker: `opendatalab/mineru:latest` and `opendatalab/mineru:china-latest`.

## Domestic AI chips (China-focused)

3.0+ supports inference on a long list of non-NVIDIA accelerators:
Ascend, Cambricon, Enflame, MetaX, Moore Threads, Kunlunxin, Iluvatar,
Hygon, Biren, T-Head. This is the major reason MinerU has so many env
vars for device selection — different vendor stacks need different
torch/cuDNN/MPS workarounds.

The path: install vendor-specific torch (e.g. `torch-npu` for Ascend),
set `MINERU_DEVICE_MODE=npu:0`, fall back to `transformers` engine when
vendor doesn't have a vLLM/LMDeploy port.

## Custom hosted (mineru.net)

The team runs a managed version at https://mineru.net with:
- $5 free credits on signup.
- Higher-accuracy "Pro" model (the same `MinerU2.5-Pro-2604-1.2B` plus
  proprietary post-processing).
- 200M+ pages/week throughput.
- SOC 2 Type 2 / BAAs / zero-retention by default.

For your hobby use case (`017-ocr-works`), the OSS pipeline + a local
GPU (or rented one) covers everything. The hosted tier is for production
volume.

## When NOT to use MinerU

Worth saying explicitly: if your only goal is converting a single PDF to
markdown for personal use, MinerU's full stack is overkill compared to
marker or chandra. The MinerU value proposition kicks in when:

- You need DOCX/PPTX/XLSX support too.
- You want a long-running API server.
- You want multi-GPU orchestration.
- You want to plug into an existing RAG framework via a named integration.
- You need 109-language OCR explicitly with per-language packs.

For a one-off `Tricks of the 3D Game Programming Gurus.pdf` → markdown
conversion, marker + your existing `run.py` is simpler and works on a
6 GB GPU.

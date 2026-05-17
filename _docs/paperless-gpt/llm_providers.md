# LLM Providers (for titling / tagging)

The OCR layer ([ocr_providers.md](ocr_providers.md)) is separate from
the **content LLM** that handles titles, tags, correspondents, dates,
custom fields, and ad-hoc analysis. Set with `LLM_PROVIDER` / `LLM_MODEL`.

paperless-gpt uses [`langchaingo`](https://github.com/tmc/langchaingo)
as the abstraction, so all five providers go through one interface.

## openai

```yaml
LLM_PROVIDER: "openai"
LLM_MODEL: "gpt-4o"
OPENAI_API_KEY: "sk-..."
# Optional Azure-OpenAI variant:
# OPENAI_API_TYPE: "azure"
# OPENAI_BASE_URL: "https://your-resource.openai.azure.com"
```

## ollama (local)

```yaml
LLM_PROVIDER: "ollama"
LLM_MODEL: "qwen3:8b"
OLLAMA_HOST: "http://host.docker.internal:11434"
OLLAMA_CONTEXT_LENGTH: "8192"     # NumCtx, default = model default
TOKEN_LIMIT: "1000"               # cap content size for small models
```

For OCR via Ollama vision models, see
[ocr_providers.md](ocr_providers.md#1-llm-default) and use
`OLLAMA_OCR_TOP_K` for sampling control.

## mistral

```yaml
LLM_PROVIDER: "mistral"
LLM_MODEL: "mistral-large-latest"
MISTRAL_API_KEY: "..."
```

## anthropic

```yaml
LLM_PROVIDER: "anthropic"
LLM_MODEL: "claude-sonnet-4-5"
ANTHROPIC_API_KEY: "..."
```

## googleai (Gemini)

```yaml
LLM_PROVIDER: "googleai"
LLM_MODEL: "gemini-2.0-flash"
GOOGLEAI_API_KEY: "..."
# GOOGLEAI_THINKING_BUDGET: "0"   # disable thinking on supported models
```

## Throttling

Same knobs for every provider — useful for free-tier limits or local
Ollama on a single GPU:

| Env | Default | Meaning |
|-----|---------|---------|
| `LLM_REQUESTS_PER_MINUTE` | 120 | RPM ceiling |
| `LLM_MAX_RETRIES` | 3 | Retry count on 429/5xx |
| `LLM_BACKOFF_MAX_WAIT` | 30s | Max wait between retries |

Vision-LLM OCR has its own parallel knobs:
`VISION_LLM_REQUESTS_PER_MINUTE`, `VISION_LLM_MAX_RETRIES`,
`VISION_LLM_BACKOFF_MAX_WAIT`, `VISION_LLM_MAX_TOKENS`,
`VISION_LLM_TEMPERATURE`.

> For GPT-5 the docs note `VISION_LLM_TEMPERATURE` *must* be explicitly
> set to `1.0` — that's the only value the model accepts.

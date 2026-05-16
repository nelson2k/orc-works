# Services (`marker/services/`)

LLM backends. Used only when `use_llm=True` (via a CLI flag, `--use_llm`, or
config). A converter resolves one service and stashes it in
`artifact_dict["llm_service"]` so any processor / extractor can grab it.

## `BaseService`

Shared knobs: `timeout=30`, `max_retries=2`, `retry_wait_time=3`,
`max_output_tokens=None`. Provides:

- `img_to_base64(img, format="WEBP")`.
- `process_images(images)` — subclass-specific image encoding into provider
  parts.
- `format_image_for_llm(image)` — accepts a single image or a list.
- `__call__(prompt, image, block, response_schema, ...)` — abstract; every
  service must return a dict that fits `response_schema`.

On construction `verify_config_keys(self)` complains if required fields (API
keys, etc.) are missing.

## Built-ins

| Module             | Class                | Notes |
|--------------------|----------------------|-------|
| `gemini.py`        | `BaseGeminiService` + `GoogleGeminiService` | Default service. Uses `google.genai`. Model `gemini-2.0-flash`. Configure with `gemini_api_key` (or `GOOGLE_API_KEY`). Supports `thinking_budget`. |
| `vertex.py`        | `GoogleVertexService`| Subclass of `BaseGeminiService` that talks to Vertex AI. Needs `vertex_project_id`, optional `vertex_location` (default `us-central1`) and `vertex_dedicated`. |
| `claude.py`        | `ClaudeService`      | Anthropic API. `claude_api_key`, `claude_model_name` (default `claude-3-7-sonnet-20250219`), `max_claude_tokens=8192`. |
| `openai.py`        | `OpenAIService`      | Any OpenAI-compatible endpoint. `openai_base_url`, `openai_model` (default `gpt-4o-mini`), `openai_api_key`, `openai_image_format`. |
| `azure_openai.py`  | `AzureOpenAIService` | `azure_endpoint`, `azure_api_key`, `deployment_name`. |
| `ollama.py`        | `OllamaService`      | Local. `ollama_base_url` (default `http://localhost:11434`), `ollama_model` (default `llama3.2-vision`). |

## Picking a service from the CLI

```bash
--use_llm \
--llm_service marker.services.claude.ClaudeService \
--claude_api_key sk-…
```

`ConfigParser.get_llm_service()` returns the dotted path; the converter then
imports and resolves it via `resolve_dependencies` so per-service config is
auto-assigned from `assign_config`.

## Structured responses

All services accept a `response_schema: type[BaseModel]` and return a parsed
dict. Per-block prompts (LLM-simple processors) and structured extraction
both rely on this contract — see [processors_llm.md](processors_llm.md) and
[extractors.md](extractors.md).

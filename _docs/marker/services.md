# LLM services

[marker/services/](../../repos-folder/marker/marker/services/). Active only when `use_llm=True` (or an LLM-only converter like `ExtractionConverter`).

Base class [services/__init__.py](../../repos-folder/marker/marker/services/__init__.py):

- `timeout=30`, `max_retries=2`, `retry_wait_time=3`, `max_output_tokens=None`.
- `__call__(prompt, image, block, response_schema, …)` returns a parsed pydantic model — every service must coerce to `response_schema`.
- `img_to_base64`, `process_images(images) -> parts` — each backend converts images to its own part type.
- `verify_config_keys` in `BaseService.__init__` errors out at construction if a required key (e.g. API key) is missing.

## Available backends

| Backend | Module | Default model | Required config |
|---|---|---|---|
| Gemini (Google AI) | `marker.services.gemini.GoogleGeminiService` | `gemini-2.0-flash` | `gemini_api_key` (or env `GOOGLE_API_KEY`) |
| Vertex AI | `marker.services.vertex.GoogleVertexService` | gemini via Vertex | `vertex_project_id` |
| Ollama | `marker.services.ollama.OllamaService` | configurable | `ollama_base_url`, `ollama_model` |
| Anthropic Claude | `marker.services.claude.ClaudeService` | configurable | `claude_api_key`, `claude_model_name` |
| OpenAI-compatible | `marker.services.openai.OpenAIService` | configurable | `openai_api_key`, `openai_model`, `openai_base_url` |
| Azure OpenAI | `marker.services.azure_openai.AzureOpenAIService` | configurable | `azure_endpoint`, `azure_api_key`, `deployment_name` |

## Selecting

CLI: `--llm_service marker.services.claude.ClaudeService --claude_api_key … --use_llm`.

Python: pass `llm_service="marker.services.openai.OpenAIService"` to `PdfConverter`. The converter resolves it through `resolve_dependencies` and stuffs it into `artifact_dict["llm_service"]` so every processor that needs it can pull it out.

## Where LLM calls happen

Every `LLMxxxProcessor` in [processors/llm/](../../repos-folder/marker/marker/processors/llm/). They produce a `PromptData` (`prompt`, `image`, `block`, `schema`) and the `LLMSimpleBlockMetaProcessor` ([processors/llm/llm_meta.py](../../repos-folder/marker/marker/processors/llm/llm_meta.py)) dispatches them through a `ThreadPoolExecutor` with `max_concurrency` (default 3) workers. Token usage is recorded on `block.metadata` and surfaces in `page_stats`.

## Cost knob

`use_llm` switches LLM processors on. Many of them have additional kill-switches (e.g. `disable_image_description`, `llm_table_correction`). When wiring marker into our app, exposing a single "boost quality with LLM" toggle plus per-feature toggles is enough.

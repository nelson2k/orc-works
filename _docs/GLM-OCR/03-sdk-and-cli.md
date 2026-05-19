# SDK And CLI

## Main SDK Entry Point

The primary Python class is `GlmOcr` in `glmocr/api.py`.

It supports:

- Direct Python use.
- Context manager lifecycle.
- Single input parsing.
- List input parsing.
- Streaming result generation.
- MaaS and self-hosted modes.
- Raw MaaS API calls.

Example shape represented in the code:

```python
from glmocr import GlmOcr

with GlmOcr(api_key="sk-xxx", mode="maas") as parser:
    result = parser.parse("document.pdf")
    print(result.markdown_result)
    print(result.to_dict())
```

There is also a convenience function named `parse` in `glmocr/api.py` that creates `GlmOcr`, parses, and closes resources.

## Accepted Inputs

The SDK accepts:

- `str` local file path.
- `pathlib.Path`.
- Raw `bytes` for image or PDF content.
- `file://` URL.
- `http://` or `https://` URL.
- `data:` URI.
- A list mixing these types.

The CLI supports files and directories with these suffixes:

- `.jpg`
- `.jpeg`
- `.png`
- `.bmp`
- `.gif`
- `.webp`
- `.pdf`

When the CLI input is a directory, it searches recursively and preserves relative subfolder structure in the output directory.

## CLI Command

The package installs:

```bash
glmocr
```

Main subcommand:

```bash
glmocr parse <input>
```

Important CLI options:

- `--output`, `-o`: output directory, default `./output`.
- `--no-save`: print or process without saving files.
- `--no-layout-vis`: skip layout visualization output.
- `--config`, `-c`: use a YAML config file.
- `--json-only`: suppress Markdown output when printing.
- `--stdout`: print results to stdout.
- `--api-key`, `-k`: pass API key directly.
- `--mode`: `maas` or `selfhosted`.
- `--env-file`: load environment variables from a specific `.env`.
- `--log-level`: DEBUG, INFO, WARNING, or ERROR.
- `--layout-device`: `cpu`, `cuda`, or `cuda:N`.
- `--set KEY VALUE`: override config values with dotted paths.

The CLI uses a progress bar and, in self-hosted mode, can show queue sizes as `Q1` and `Q2`.

## Configuration Priority

Configuration is loaded by `glmocr/config.py`.

Priority from highest to lowest:

1. CLI `--set` overrides.
2. Python keyword overrides.
3. Environment variables or `.env`.
4. YAML config file.
5. Built-in model defaults.

Primary environment variables:

- `ZHIPU_API_KEY`: main API key.
- `GLMOCR_API_KEY`: legacy fallback.
- `GLMOCR_MODE`: `maas` or `selfhosted`.
- `GLMOCR_API_URL`: MaaS API URL.
- `GLMOCR_MODEL`: MaaS model.
- `GLMOCR_TIMEOUT`: MaaS request timeout.
- `GLMOCR_OCR_API_URL`: self-hosted OCR API URL.
- `GLMOCR_OCR_API_HOST`: self-hosted OCR API host.
- `GLMOCR_OCR_API_PORT`: self-hosted OCR API port.
- `GLMOCR_OCR_MODEL`: self-hosted OCR model.
- `GLMOCR_LAYOUT_CUDA_VISIBLE_DEVICES`: layout CUDA devices.
- `GLMOCR_LAYOUT_DEVICE`: explicit layout device.
- `GLMOCR_LOG_LEVEL`: logging level.

If an API key is present and no mode is explicitly passed, `GlmOcr` defaults to MaaS mode.

## MaaS Client

`MaaSClient` in `glmocr/maas_client.py` sends requests to the configured layout parsing endpoint.

It handles:

- API key discovery.
- HTTP session setup.
- Connection pooling.
- Retry/backoff for selected status codes.
- URL inputs.
- Local file inputs.
- Raw bytes.
- Raw base64-like strings.
- Data URI payloads.
- PDF data URI wrapping.
- JPEG/PNG validation or conversion.

The raw MaaS response may contain:

- `id`
- `created`
- `model`
- `md_results`
- `layout_details`
- `layout_visualization`
- `data_info`
- `usage`
- `request_id`

`GlmOcr` converts that response to a `PipelineResult`.

## Self-Hosted OCR Client

`OCRClient` in `glmocr/ocr_client.py` calls a configured recognition API.

It supports two API modes:

- `openai`: OpenAI-compatible `/v1/chat/completions`.
- `ollama_generate`: Ollama native `/api/generate`.

It handles:

- URL construction from host/port/scheme/path.
- Optional API key bearer auth.
- Extra headers.
- SSL verification setting.
- Socket-level liveness checks.
- Startup connection testing.
- Retry/backoff for transient failures.
- Response normalization into an OpenAI-like `choices[0].message.content` shape.

## Result Saving

`PipelineResult.save()` saves:

- JSON result: `<output>/<stem>/<stem>.json`
- Markdown result: `<output>/<stem>/<stem>.md`
- Raw model JSON if present: `<output>/<stem>/<stem>_model.json`
- Cropped images if present: `<output>/<stem>/imgs/...`
- Layout visualization if present: `<output>/<stem>/layout_vis/...`

File and folder stems are sanitized for Windows-invalid path characters.

## Result Serialization

`PipelineResult.to_dict()` returns:

- `json_result`
- `markdown_result`
- `original_images`
- Optional `usage`
- Optional `data_info`
- Optional `error`

`PipelineResult.to_json()` serializes that dictionary with Unicode preserved and indentation by default.

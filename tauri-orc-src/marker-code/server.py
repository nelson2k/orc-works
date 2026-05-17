"""Minimal FastAPI server that exposes marker as an HTTP endpoint.

Run standalone:
    venv/Scripts/python.exe -m uvicorn server:app --host 127.0.0.1 --port 7423

The Tauri app at tauri-orc-src/src-tauri spawns this same command as a child process.
"""

import os
import tempfile
from pathlib import Path

from fastapi import FastAPI, File, Form, HTTPException, UploadFile
from fastapi.responses import JSONResponse

app = FastAPI(title="marker-server")

# Low-VRAM preset (mirrors the legacy run.py defaults — safe for a 6 GB card).
# Sent only when the client opts out of --full-vram.
LOW_VRAM_BATCH_SIZES = {
    "layout_batch_size": 6,
    "detection_batch_size": 2,
    "recognition_batch_size": 16,
    "table_rec_batch_size": 4,
    "equation_batch_size": 4,
}

OPENAI_LLM_SERVICE = "marker.services.openai.OpenAIService"

_artifact_dict = None


def get_artifact_dict():
    """Lazy-load the 5 surya models once. Shared across all requests."""
    global _artifact_dict
    if _artifact_dict is None:
        from marker.models import create_model_dict

        _artifact_dict = create_model_dict()
    return _artifact_dict


def parse_bool(value: str) -> bool:
    return value.strip().lower() in {"true", "1", "yes", "on"}


def build_config(
    page_range: str,
    model: str,
    use_llm: bool,
    full_vram: bool,
) -> dict:
    cli_opts: dict = {"use_llm": use_llm}
    if page_range:
        cli_opts["page_range"] = page_range
    if use_llm:
        key = os.environ.get("OPENAI_API_KEY")
        if not key:
            raise HTTPException(
                status_code=500,
                detail=(
                    "OPENAI_API_KEY is not set; either export it in the environment "
                    "(e.g. via marker-code/local.env) or send no_llm=true."
                ),
            )
        cli_opts["openai_api_key"] = key
        cli_opts["openai_model"] = model
    if not full_vram:
        cli_opts.update(LOW_VRAM_BATCH_SIZES)

    from marker.config.parser import ConfigParser

    return ConfigParser(cli_opts).generate_config_dict()


@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/convert")
async def convert(
    file: UploadFile = File(...),
    page_range: str = Form(""),
    model: str = Form("gpt-4o-mini"),
    no_llm: str = Form("false"),
    full_vram: str = Form("false"),
):
    use_llm = not parse_bool(no_llm)
    full_vram_on = parse_bool(full_vram)

    config = build_config(
        page_range=page_range,
        model=model,
        use_llm=use_llm,
        full_vram=full_vram_on,
    )

    from marker.converters.pdf import PdfConverter
    from marker.output import text_from_rendered

    converter = PdfConverter(
        artifact_dict=get_artifact_dict(),
        config=config,
        llm_service=OPENAI_LLM_SERVICE if use_llm else None,
    )

    with tempfile.NamedTemporaryFile(suffix=".pdf", delete=False) as tmp:
        tmp.write(await file.read())
        tmp_path = tmp.name
    try:
        rendered = converter(tmp_path)
        text, _, _ = text_from_rendered(rendered)
        return JSONResponse({"markdown": text})
    finally:
        Path(tmp_path).unlink(missing_ok=True)

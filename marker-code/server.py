"""Minimal FastAPI server that exposes marker as an HTTP endpoint.

Run standalone:
    venv/Scripts/python.exe -m uvicorn server:app --host 127.0.0.1 --port 7423

The Tauri app at src/src-tauri spawns this same command as a child process.
"""

import tempfile
from pathlib import Path

from fastapi import FastAPI, File, UploadFile
from fastapi.responses import JSONResponse

app = FastAPI(title="marker-server")

_converter = None


def get_converter():
    global _converter
    if _converter is None:
        from marker.converters.pdf import PdfConverter
        from marker.models import create_model_dict

        _converter = PdfConverter(artifact_dict=create_model_dict())
    return _converter


@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/convert")
async def convert(file: UploadFile = File(...)):
    converter = get_converter()
    with tempfile.NamedTemporaryFile(suffix=".pdf", delete=False) as tmp:
        tmp.write(await file.read())
        tmp_path = tmp.name
    try:
        rendered = converter(tmp_path)
        from marker.output import text_from_rendered

        text, _, _ = text_from_rendered(rendered)
        return JSONResponse({"markdown": text})
    finally:
        Path(tmp_path).unlink(missing_ok=True)

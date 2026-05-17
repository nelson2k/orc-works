from pathlib import Path

from fastapi import FastAPI, File, Form, HTTPException, UploadFile
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse

from .marker_service import ConversionOptions, convert_pdf

PROJECT_ROOT = Path(__file__).resolve().parents[2]
OUTPUT_ROOT = PROJECT_ROOT / "output" / "web"
UPLOAD_ROOT = PROJECT_ROOT / "backend" / "data" / "uploads"

app = FastAPI(title="OCR Works API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173", "http://127.0.0.1:5173"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/api/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/api/convert")
async def convert(
    pdf: UploadFile = File(...),
    no_llm: bool = Form(False),
    model: str = Form("gpt-4o-mini"),
    page_range: str | None = Form(None),
    full_vram: bool = Form(False),
) -> dict[str, str]:
    if not pdf.filename or not pdf.filename.lower().endswith(".pdf"):
        raise HTTPException(status_code=400, detail="Upload a PDF file.")

    UPLOAD_ROOT.mkdir(parents=True, exist_ok=True)
    safe_name = Path(pdf.filename).name
    upload_path = UPLOAD_ROOT / safe_name
    upload_path.write_bytes(await pdf.read())

    options = ConversionOptions(
        pdf_path=upload_path,
        output_root=OUTPUT_ROOT,
        use_llm=not no_llm,
        model=model,
        page_range=page_range.strip() or None if page_range else None,
        full_vram=full_vram,
    )

    try:
        result = convert_pdf(options)
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc

    return {
        "documentName": result.document_name,
        "markdown": result.markdown,
        "markdownUrl": f"/api/files/{result.document_name}/{result.markdown_file.name}",
        "metadataUrl": f"/api/files/{result.document_name}/{result.metadata_file.name}",
    }


@app.get("/api/files/{document_name}/{file_name}")
def get_output_file(document_name: str, file_name: str) -> FileResponse:
    doc_dir = (OUTPUT_ROOT / document_name).resolve()
    file_path = (doc_dir / file_name).resolve()
    if OUTPUT_ROOT.resolve() not in file_path.parents or not file_path.exists():
        raise HTTPException(status_code=404, detail="File not found.")
    return FileResponse(file_path)

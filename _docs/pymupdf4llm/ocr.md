# OCR

OCR is selective by default. The parser analyzes each PDF page and invokes OCR only when the page appears to need it.

OCR mode enum in `src/ocr/__init__.py`:

- `NEVER`
- `SELECT_REMOVING_OLD`
- `SELECT_PRESERVING_OLD`
- `ALWAYS_REMOVING_OLD`
- `ALWAYS_PRESERVING_OLD`

Default layout mode uses `SELECT_REMOVING_OLD`. `force_ocr=True` switches to `ALWAYS_REMOVING_OLD`.

Page analysis lives in `helpers/utils.py::analyze_page`. It checks signals such as:

- unreadable replacement characters
- image-covered regions that look text-like
- suspicious vector graphics
- prior OCR/invisible text spans

OCR callback selection lives in `helpers/document_layout.py::select_ocr_function`.

Selection preference:

1. Tesseract tessdata plus `rapidocr_onnxruntime`: `rapidtess_api.exec_ocr`
2. Tesseract tessdata plus Paddle-compatible availability: `paddletess_api.exec_ocr`
3. Tesseract tessdata only: `tesseract_api.exec_ocr`
4. RapidOCR only: `rapidocr_api.exec_ocr`
5. PaddleOCR only: `paddleocr_api.exec_ocr`

The Tesseract callback uses PyMuPDF's `Pixmap.pdfocr_tobytes(...)`. RapidOCR callbacks detect/recognize regions and insert recognized text back into the PDF page as extractable text.

If OCR is enabled but no OCR function is available, OCR is disabled with a warning. If always-OCR mode is requested and no OCR function is available, parsing raises `ValueError`.


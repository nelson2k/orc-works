# Surya Scripts

Console scripts from `pyproject.toml`:

| Command | Python target | Purpose |
| --- | --- | --- |
| `surya_detect` | `surya.scripts.detect_text:detect_text_cli` | text detection |
| `surya_ocr` | `surya.scripts.ocr_text:ocr_text_cli` | OCR text |
| `surya_layout` | `surya.scripts.detect_layout:detect_layout_cli` | layout detection |
| `surya_gui` | `surya.scripts.run_streamlit_app:streamlit_app_cli` | Streamlit GUI |
| `surya_table` | `surya.scripts.table_recognition:table_recognition_cli` | table recognition |
| `surya_latex_ocr` | `surya.scripts.ocr_latex:ocr_latex_cli` | LaTeX OCR |
| `texify_gui` | `surya.scripts.run_texify_app:texify_app_cli` | Texify GUI |

The repo root also has tiny wrapper files such as `detect_layout.py`,
`detect_text.py`, `ocr_text.py`, `table_recognition.py`, and `ocr_latex.py`.

## Debug output

Benchmark scripts accept `--debug` options that render detected boxes. The debug
drawing helpers live in `surya/debug/`.

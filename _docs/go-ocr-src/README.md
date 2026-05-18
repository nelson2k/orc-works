# go-ocr-src

Notes on the `go-ocr-src/` folder.

This folder contains a Windows-oriented desktop OCR application. The GUI is
written in Go with Fyne. The OCR and PDF processing work happens in a
long-lived Python worker process.

The app opens a PDF, renders pages for preview, and extracts Markdown from a
single page or every page. Extraction can use several engines:

- digital text extraction with `pymupdf4llm`
- Marker OCR
- Marker OCR with an OpenAI-compatible local LLM service
- Qwen2.5-VL visual-language extraction
- automatic routing across the above paths

The Go and Python processes communicate through newline-delimited JSON over
stdin/stdout.

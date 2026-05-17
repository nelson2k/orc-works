# paperless-gpt — notes

Source: `repos-folder/paperless-gpt/` (icereed/paperless-gpt, MIT).

paperless-gpt is a **companion service for paperless-ngx** — not a
standalone OCR tool. It hooks into paperless-ngx's tag system to use LLMs
for titling, tagging, correspondent detection, custom field extraction,
and (optionally) re-OCRing scans with a vision LLM that beats vanilla
Tesseract on messy documents.

Written in Go (binary) + React/Vite frontend (single Docker image).

## Files in this folder

- [overview.md](overview.md) — what it does, who it's for
- [architecture.md](architecture.md) — Go binary, frontend, paperless-ngx coupling
- [ocr_providers.md](ocr_providers.md) — 5 OCR backends (LLM / Azure / Google / Mistral / Docling)
- [ocr_modes.md](ocr_modes.md) — image / pdf / whole_pdf processing modes
- [llm_providers.md](llm_providers.md) — OpenAI / Ollama / Mistral / Anthropic / GoogleAI for the *titling* LLM
- [prompts.md](prompts.md) — template system and variables
- [enhanced_pdf.md](enhanced_pdf.md) — hOCR-based searchable PDF generation
- [tags_and_workflow.md](tags_and_workflow.md) — manual vs auto tag-driven flow
- [config.md](config.md) — env vars cheat-sheet
- [install.md](install.md) — docker-compose recipe
- [vs_others.md](vs_others.md) — vs marker / MinerU / chandra (and why it's not really a competitor)

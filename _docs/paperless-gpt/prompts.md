# Prompt Templates

paperless-gpt ships 8 Go `text/template` files in
[default_prompts/](../../repos-folder/paperless-gpt/default_prompts/).
On first run they're copied to `prompts/` (mount this as a volume to
persist edits). All edits in the web UI's Settings page write to
`prompts/`; templates are hot-reloaded on save.

## The 8 templates

| File | Used for |
|------|----------|
| `ocr_prompt.tmpl` | LLM-based OCR (vision model transcribing the page image) |
| `title_prompt.tmpl` | Suggested document title |
| `tag_prompt.tmpl` | Tags (filtered/optionally extended) |
| `correspondent_prompt.tmpl` | Sender/issuer name |
| `created_date_prompt.tmpl` | Document date extraction |
| `document_type_prompt.tmpl` | Pick from paperless-ngx document types |
| `custom_field_prompt.tmpl` | Fill arbitrary paperless-ngx custom fields |
| `adhoc-analysis_prompt.tmpl` | User-supplied prompt across N documents |

## Variables per template

**ocr_prompt** — `{{.Language}}` only (the image is sent separately).

**title_prompt** — `{{.Language}}`, `{{.Content}}`, `{{.Title}}` (original).

**tag_prompt** — `{{.Language}}`, `{{.AvailableTags}}`, `{{.OriginalTags}}`,
`{{.Title}}`, `{{.Content}}`, `{{.CreateNewTags}}`.

**correspondent_prompt** — `{{.Language}}`, `{{.AvailableCorrespondents}}`,
`{{.BlackList}}`, `{{.Title}}`, `{{.Content}}`.

**created_date_prompt** — `{{.Language}}`, `{{.Content}}`.

**custom_field_prompt** — `{{.DocumentType}}`, `{{.CustomFieldsXML}}`,
`{{.Title}}`, `{{.CreatedDate}}`, `{{.Content}}`.

## Default OCR prompt

The default OCR prompt is striking-low-tech ([source](../../repos-folder/paperless-gpt/default_prompts/ocr_prompt.tmpl)):

```
Just transcribe the text in this image and preserve the formatting and layout (high quality OCR).
Do that for ALL the text in the image. Be thorough and pay attention. This is very important.
The image is from a text document so be sure to continue until the bottom of the page.
Thanks a lot! You tend to forget about some text in the image so please focus! Use markdown format but without a code block.
```

Yes, the prompt politely thanks the model. This matters more than you'd
expect — the "you tend to forget some text, please focus" line is real
prompt engineering that catches truncation failures.

## Customisation flow

1. **Don't edit `default_prompts/`** — it's read-only.
2. Edit via web UI Settings page (saves to `prompts/`), or hand-edit
   files under `prompts/` if you mounted that volume.
3. paperless-gpt watches `prompts/` and reloads templates on change —
   no restart needed.

## Sprig functions available

The templates use [`sprig/v3`](https://github.com/Masterminds/sprig)
funcs (e.g. `{{.AvailableTags | join ", "}}` in `tag_prompt.tmpl`).

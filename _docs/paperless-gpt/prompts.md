# Prompt templates

Every LLM call paperless-gpt makes is rendered from a Go `text/template` with [sprig](https://masterminds.github.io/sprig/) functions. The templates ship in `default_prompts/` (read-only baseline) and are copied to `prompts/` on first run; the running `prompts/` copy is what's actually used. Users edit them through the Settings page in the UI, which writes back to `prompts/<name>.tmpl` and reloads them immediately. Mount `./prompts:/app/prompts` to persist edits across container restarts.

Loading is implemented in `loadTemplates` in [main.go](../../repos-folder/paperless-gpt/main.go).

## Files

```
default_prompts/
├── adhoc-analysis_prompt.tmpl
├── correspondent_prompt.tmpl
├── created_date_prompt.tmpl
├── custom_field_prompt.tmpl
├── document_type_prompt.tmpl
├── ocr_prompt.tmpl
├── tag_prompt.tmpl
└── title_prompt.tmpl
```

## Template variables

| Template | Variables |
|---|---|
| `title_prompt.tmpl` | `Language`, `Content`, `Title` |
| `tag_prompt.tmpl` | `Language`, `AvailableTags`, `OriginalTags`, `Title`, `Content` |
| `ocr_prompt.tmpl` | `Language` |
| `correspondent_prompt.tmpl` | `Language`, `AvailableCorrespondents`, `BlackList`, `Title`, `Content` |
| `created_date_prompt.tmpl` | `Language`, `Content` |
| `document_type_prompt.tmpl` | `Language`, `AvailableDocumentTypes`, `Title`, `Content` |
| `custom_field_prompt.tmpl` | `DocumentType`, `CustomFieldsXML`, `Title`, `CreatedDate`, `Content` |
| `adhoc-analysis_prompt.tmpl` | `Documents`, `Prompt` |

`Language` always comes from `LLM_LANGUAGE` (default `English`), title-cased.

`AvailableTags`, `AvailableCorrespondents`, `AvailableDocumentTypes` are lists fetched from paperless-ngx. Templates typically render them as comma-separated lists or bullet lists using sprig's `join` and `range`.

`OriginalTags` is the document's current tag list — used by `tag_prompt.tmpl` to decide which tags to keep, remove, or add.

`BlackList` (`CORRESPONDENT_BLACK_LIST`) prevents the LLM from suggesting specific correspondents — useful for excluding your own organization name from being treated as a correspondent.

`CustomFieldsXML` is an XML serialization of the custom fields selected in the Settings page, including their data types. Letting the model see the schema reduces hallucination.

## Sprig

The full sprig function set is available because `template.New(name).Funcs(sprig.FuncMap())` is used. Common functions templates rely on: `join`, `default`, `lower`, `upper`, `quote`, `replace`, `trim`, `regexFind`, `now`, `date`, `printf`. See <https://masterminds.github.io/sprig/> for the catalog.

## Editing through the UI

`GET /api/prompts` returns the current contents of every file in `prompts/` as a JSON object keyed by filename. `POST /api/prompts` accepts the same shape and writes back, then triggers a reload (`loadTemplates`). No restart needed.

Errors in a saved template will surface on the next LLM call — paperless-gpt will log the parse error and skip that field. If you break things, deleting the file from `prompts/` and restarting the container restores the default.

## Adding new fields

The set of prompts is hard-coded — they're loaded by name in `loadTemplates` and referenced as `titleTemplate`, `tagTemplate`, etc. in Go code. To add a new prompt, you'd need to:

1. Add the `.tmpl` file in both `default_prompts/` and `prompts/`.
2. Declare a new package-level `*template.Template` variable and load it in `loadTemplates`.
3. Add Go code that consumes it and a handler that calls that code.

There's no plugin mechanism — templates are infrastructure, not data.

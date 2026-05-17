# Tag-driven Workflow

paperless-gpt has no scheduler — everything is triggered by tags you
attach to documents inside paperless-ngx. A background poller
(`background.go`) watches for matching tags and queues jobs.

## The four trigger tags

| Env var | Default tag | What happens |
|---------|-------------|--------------|
| `MANUAL_TAG` | `paperless-gpt` | Doc shows up in paperless-gpt's web UI for **manual** review |
| `AUTO_TAG` | `paperless-gpt-auto` | Title/tags/correspondent **auto-applied** |
| (OCR manual)| `paperless-gpt-ocr` | Re-OCR queued for manual review |
| `AUTO_OCR_TAG` | `paperless-gpt-ocr-auto` | Re-OCR auto-applied |

You apply tags by hand in paperless-ngx, or via paperless-ngx's own
auto-tagger rules (e.g. tag every document from "Vodafone" with
`paperless-gpt-auto`).

## Manual review flow

1. You tag doc with `paperless-gpt`.
2. paperless-gpt fetches it, runs the title/tag/correspondent prompts.
3. Doc appears in the web UI at `http://localhost:8080`.
4. You click **Generate Suggestions** → see proposed values.
5. Edit if needed → **Apply** → paperless-gpt PATCHes the doc in
   paperless-ngx and removes the trigger tag.

## Auto-apply flow

1. You tag doc with `paperless-gpt-auto`.
2. paperless-gpt processes it without showing a review screen and
   writes results back immediately.
3. Trigger tag is removed. Done.

Which fields get auto-generated is controlled by these env vars (all
default `true`):

- `AUTO_GENERATE_TITLE`
- `AUTO_GENERATE_TAGS`
- `AUTO_GENERATE_CORRESPONDENTS`
- `AUTO_GENERATE_DOCUMENT_TYPE`
- `AUTO_GENERATE_CREATED_DATE`

## Tag-creation control

By default the LLM can only **suggest from existing tags** in
paperless-ngx. To let it invent new ones:

```yaml
CREATE_NEW_TAGS: "true"
```

When enabled, suggested-but-missing tags are created in paperless-ngx
automatically.

## Correspondent blacklist

```yaml
CORRESPONDENT_BLACK_LIST: "John Doe, Jane Smith"
```

Names in this comma-separated list are filtered out of the LLM's
correspondent suggestions — handy when the model keeps proposing your
own name or some boilerplate signature.

## Custom fields

Disabled by default. Enable in the web UI Settings page, pick which
custom fields to populate, and choose the write mode:

- **append**: only add fields that aren't already set (safest)
- **update**: overwrite existing fields with new suggestions
- **replace**: delete all existing custom fields, replace with the
  suggested set (destructive)

The `custom_field_prompt.tmpl` is sent one document at a time with the
selected fields rendered as XML so the LLM can produce structured
output.

## Ad-hoc analysis

Separate flow from the tag pipeline: in the web UI, select N
documents, type a prompt, get back a single LLM response across all
of them. Uses `adhoc-analysis_prompt.tmpl`.

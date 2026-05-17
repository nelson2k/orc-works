# Links and cross-references (`pdftext/pdf/links.py`)

Reads PDF link annotations and stitches them into the span tree. Triggered
by `dictionary_output(disable_links=False)` (the default). Skipped entirely
in the plain-text path.

## `get_links(page_idx, pdf) → list[Link]`

Iterates `FPDFPage_GetAnnotCount` and keeps only `FPDF_ANNOT_LINK` annots.
For each link annotation:

1. Read its rectangle (`FPDFAnnot_GetRect`), translate from PDF
   bottom-left coords to top-left, rotate by page rotation → `link["bbox"]`.
2. Try `FPDFLink_GetDest` for an explicit destination. If present:
   - `link["dest_page"] = FPDFDest_GetDestPageIndex(...)`
   - `link["dest_pos"] = (x, y)` if `FPDFDest_GetLocationInPage` succeeds.
3. Else try the action: `FPDFLink_GetAction` →
   - `PDFACTION_GOTO` → same as above (internal jump).
   - `PDFACTION_URI` → decode bytes, `link["url"] = "https://…"`.
   - `PDFACTION_UNSUPPORTED` → drop the link.

Returns a `list[Link]`, see [schema.md](schema.md).

## `add_links_and_refs(pages, pdf)`

For each page: `merge_links(page, pdf, refs)`. Then a final loop attaches
`page["refs"] = refs.get_refs(page["page"])` for every page (so the
destination page advertises its anchor points).

## `merge_links` — splitting spans on link bboxes

This is the interesting part. A single span (same font/size/etc.) can be
half-link, half-not — e.g. `"see Figure 3 for details"` where only `Figure
3` is hyperlinked. The flow:

1. Collect all spans on the page → `span_bboxes`.
2. `matrix_intersection_area(link_bboxes, span_bboxes)` (numpy) gives an
   `(L, S)` overlap matrix.
3. For each link, pick the **single span with the largest overlap**. (Hover
   over multiple spans? Tough luck — the model is 1-link-per-span.)
4. If the link is an internal jump (`dest_page` set), allocate a `Reference`
   via `PageReference.add_ref(dest_page, dest_pos)`. The link's `url` is
   rewritten to `#page-N-K` (deduped per (page, coord) pair).
5. Record `span_link_map[max_intersection].append(link)`.

After bucketing, replay each line:

- Spans without any associated link are kept verbatim.
- Spans with one or more links go through `_reconstruct_spans`.

## `_reconstruct_spans(orig_span, links)`

Per character in the original span:

1. Compute intersection area between the char's bbox and every link bbox.
2. Pick the link with the largest overlap (sorted desc by area).
3. Start a new sub-span whenever the resolved `url` changes; otherwise grow
   the current sub-span.

Result: the original span is split into N consecutive sub-spans, each with
its own `url`. Empty/no-link runs get `url=""`.

This is why `dictionary_output` needs the char list and why `keep_chars=True`
or the internal `chars` field is required during this pass. The `chars`
field is removed afterwards unless the caller asked for it.

## When you don't want this

`disable_links=True` skips the whole pass — useful if:

- You only care about text content.
- Your PDF has thousands of links and you're CPU-bound (`matrix_intersection_area`
  is cheap but link annotations themselves are slow to enumerate).
- You're calling `table_output` and don't need link info inside cells.

The CLI's `--json` mode passes `disable_links=True` because the CLI doesn't
expose a flag to disable it (it just always skips link resolution to keep
the JSON output deterministic and fast). The Python API leaves it on by
default.

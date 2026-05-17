# Limitations

## 1. Subset fonts — you get glyph fragments, not the whole alphabet

PDFs almost always embed **subsets** of fonts: only the glyphs actually used
in the document, with a 6-letter random prefix on the name
(`ABCDEE+Arial.ttf`, `ABCEEE+Arial,Italic.ttf` in the sample output).

Implications:

- Typing characters not present in the original PDF will render as `.notdef`
  or empty boxes.
- Kerning tables, OpenType features, and glyph alternates are usually
  stripped down to what was needed.
- Two PDFs from the same document might produce different subsets of the
  same "font" — they're not interchangeable.

If you need the full font, you need to **find the original** somewhere else
(font foundry, OS install, Google Fonts mirror).

## 2. Non-TrueType fonts are silently skipped

See [extraction.md](extraction.md). Type 1, Type 1C (CFF), and Type 3 fonts
yield nothing. PDFBox knows about them but this tool doesn't call
`getFontFile()` / `getFontFile3()`.

If your PDF is "missing" fonts in the output, that's almost always why.

## 3. Same name = single output

`FONTNAME_MAPPER` dedups by filename string. A PDF that embeds two distinct
font streams under the same name (multi-subset documents, manual edits)
gets the first one only.

## 4. License is the user's problem

The tool just copies bytes. **Most commercial fonts forbid extraction +
redistribution.** If a PDF embeds a licensed font, pulling the TTF out and
reusing it can violate:

- The font's EULA.
- DRM bits inside the font's `OS/2` table (`fsType`).
- Adobe's PDF font-embedding terms.

The tool does not inspect `fsType` ("Restricted License Embedding", "Print &
Preview only", "Editable Embedding", "Installable Embedding"). You should:

```bash
# Use fontTools (Python) to inspect fsType after extraction
python -c "from fontTools.ttLib import TTFont; print(TTFont('font.ttf')['OS/2'].fsType)"
```

`fsType=0` → installable, no restrictions. Anything non-zero → check the
EULA before reusing.

## 5. No CLI, no batch

GUI only. To process 100 PDFs you'd either:

- Click through 100 times.
- Wrap PDFBox yourself in a CLI (~50 lines).
- Switch to a Python tool like `pikepdf` or `pdfminer.six` (see
  [vs_others.md](vs_others.md)).

## 6. Java 8 is end-of-life

The project pins Java 1.8 in spirit (PDFBox 3.0 itself requires Java 8+ but
runs on newer JVMs fine). New JDKs (17, 21) work for running but you may see
deprecation warnings if you recompile.

## 7. Output ZIP is held in memory

`ByteArrayOutputStream` + `ZipOutputStream` mean the entire ZIP is built in
RAM before being written. For typical PDFs (<10 fonts × <500 KB each) this
is trivial. A monster document with 100+ embedded fonts could be ~50 MB in
memory — fine, but not designed for hyperscale.

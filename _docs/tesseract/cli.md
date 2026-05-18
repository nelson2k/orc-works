# CLI

Primary executable: `src/tesseract.cpp`.

Basic shape:

```text
tesseract FILE OUTPUTBASE [OPTIONS]... [CONFIGFILE]...
```

Important arguments:

- `FILE` can be an image, `stdin` / `-`, or a text file listing input images.
- `OUTPUTBASE` is a basename; output extensions are selected by configs or defaults.
- `-l LANG[+LANG]` selects language or script models. If omitted, `eng` is assumed.
- `--tessdata-dir PATH` points at the `tessdata` directory.
- `--dpi N` supplies image DPI when metadata is missing or wrong.
- `--psm N` selects page segmentation assumptions.
- `--oem N` selects OCR engine mode when legacy support is built.
- `-c VAR=VALUE` sets Tesseract parameters.

Useful config files in `tessdata/configs`:

- `txt` - plain text.
- `hocr` - hOCR HTML.
- `pdf` - searchable PDF.
- `tsv` - tab-separated OCR data.
- `alto` - ALTO XML.
- `page` - PAGE XML.
- `makebox`, `lstmbox`, `linebox`, `wordstrbox` - box/training-oriented outputs.
- `quiet` - suppress debug output.
- `lstm.train` - emit `.lstmf` files used for LSTM training.

Multiple output configs can be passed at once. Example:

```text
tesseract image.png out -l eng --psm 6 hocr pdf txt
```

Page segmentation modes are defined in `include/tesseract/publictypes.h` and printed by `--help-psm`. Common values are `3` for automatic layout, `6` for a single block, `7` for a single line, and `11` for sparse text.


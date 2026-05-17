# Surya Overview

Source: `repos-folder/surya`.

## Package

- Project name: `surya-ocr`
- Version in checkout: `0.17.1`
- License: GPL-3.0-or-later
- Python: `^3.10`
- Description: OCR, layout, reading order, and table recognition in 90+ languages.

## Main capabilities

From the README and code layout:

- line-level text detection
- OCR recognition
- layout analysis
- reading order detection
- table recognition
- LaTeX OCR
- OCR error detection

## Main folders

- `surya/detection/` - text line detection.
- `surya/recognition/` - OCR recognition and text lines/chars.
- `surya/layout/` - layout boxes and labels.
- `surya/table_rec/` - table cells, rows, columns.
- `surya/ocr_error/` - OCR quality/error predictor.
- `surya/foundation/` - shared foundation predictor.
- `surya/input/` - input loading/processing.
- `surya/debug/` - drawing/rendering helpers.
- `surya/scripts/` - CLI command implementations.

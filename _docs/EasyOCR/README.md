# EasyOCR - notes

Source: `repos-folder/EasyOCR`. Package version in this checkout: `1.7.2`. License: Apache-2.0.

EasyOCR is a PyTorch OCR package. Its public API is centered on:

```python
import easyocr

reader = easyocr.Reader(["en"])
result = reader.readtext("image.jpg")
```

The runtime pipeline is:

```text
input image
  -> text detection with CRAFT or DBNet
  -> box grouping and image crops
  -> text recognition with CRNN-style recognizer
  -> CTC decoding
  -> optional paragraph grouping / output formatting
```

It supports many language codes by combining packaged character lists, packaged word dictionaries, and downloadable detector/recognizer model weights.

Good first files:

- `easyocr/easyocr.py` - `Reader` API and end-to-end pipeline.
- `easyocr/config.py` - language groups, model metadata, download URLs, MD5 checks.
- `easyocr/detection.py` - CRAFT detector wrapper.
- `easyocr/detection_db.py` - DBNet detector wrapper.
- `easyocr/recognition.py` - recognizer loading and CTC inference.
- `easyocr/utils.py` - image input, box grouping, decoding helpers.
- `custom_model.md` - custom recognition model contract.


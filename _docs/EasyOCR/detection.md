# Detection

EasyOCR supports two detector families in this checkout:

- `craft` - default detector.
- `dbnet18` - optional DBNet detector.

`Reader.getDetectorPath()` chooses the detector wrapper, ensures model weights exist, verifies MD5 checksums, and downloads/re-downloads as needed.

CRAFT path:

```text
Reader.detect
  -> detection.get_textbox
    -> detection.test_net
      -> CRAFT forward pass
      -> craft_utils.getDetBoxes
      -> coordinate adjustment
  -> utils.group_text_box
```

CRAFT model metadata lives in `config.py` as `craft_mlt_25k.pth`.

DBNet path:

```text
Reader.detect
  -> detection_db.get_textbox
    -> DBNet resize / normalize
    -> image2hmap
    -> hmap2bbox
```

DBNet is adapted for inference only under `easyocr/DBNet`. It uses DBNet-style pixel-level text segmentation and has DCN operators under `DBNet/assets/ops/dcn`.

DBNet notes:

- `dbnet18` is exposed through `Reader(..., detect_network="dbnet18")`.
- `config.py` also lists `dbnet50`, but `Reader.support_detection_network` only includes `craft` and `dbnet18`.
- DCN operators can compile just-in-time when DBNet is first used.
- Ahead-of-time DCN compilation is documented in `easyocr/DBNet/README.md`.

Common detection knobs include `canvas_size`, `mag_ratio`, `text_threshold`, `low_text`, `link_threshold`, `slope_ths`, `ycenter_ths`, `height_ths`, `width_ths`, `add_margin`, and DBNet-specific `threshold`, `bbox_min_score`, `bbox_min_size`, `max_candidates`.


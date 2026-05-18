# API

Main class: `easyocr.Reader`.

Constructor shape:

```python
reader = easyocr.Reader(
    ["en"],
    gpu=True,
    model_storage_directory=None,
    user_network_directory=None,
    detect_network="craft",
    recog_network="standard",
    download_enabled=True,
    detector=True,
    recognizer=True,
    verbose=True,
    quantize=True,
    cudnn_benchmark=False,
)
```

Important constructor behavior:

- model files default to `~/.EasyOCR/model`
- custom recognizer files default to `~/.EasyOCR/user_network`
- `EASYOCR_MODULE_PATH` overrides the default module path; `MODULE_PATH` is a fallback
- `gpu=True` chooses CUDA, then Apple MPS, then CPU
- `gpu=False` forces CPU
- CPU mode can use dynamic quantization when `quantize=True`
- `download_enabled=False` makes missing/corrupt model files fail instead of downloading

Main methods:

- `readtext(image, ...)` - detect and recognize in one call.
- `detect(img, ...)` - return grouped text boxes.
- `recognize(img_cv_grey, horizontal_list, free_list, ...)` - recognize supplied boxes.
- `readtext_batched(images, ...)` - batch multiple same-size images.
- `setDetector(detect_network)` - switch detector after initialization.
- `setLanguageList(lang_list, model)` - update allowed language characters within a model.

`readtext` accepts file paths, URLs, bytes, PIL/numpy-style images accepted by `reformat_input`, and OpenCV arrays.

Output modes:

- `detail=1`, `output_format="standard"` - tuples of `(box, text, confidence)`.
- `detail=0` - list of strings.
- `output_format="dict"` - dictionaries.
- `output_format="json"` - JSON strings.
- `paragraph=True` - merge recognized boxes into paragraph-style output.


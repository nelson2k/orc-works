# Package Layout

Contents of `repos-folder/EasyOCR/`:

```text
easyocr/                       runtime package
  __init__.py                  exports Reader and version
  easyocr.py                   Reader class and OCR pipeline
  cli.py                       console entry point
  config.py                    languages, model URLs, checksums
  detection.py                 CRAFT detection wrapper
  detection_db.py              DBNet detection wrapper
  recognition.py               recognition inference helpers
  utils.py                     CTC decoding, image prep, grouping, downloads
  craft.py / craft_utils.py    CRAFT network and post-processing
  model/                       recognizer model definitions
  DBNet/                       DBNet inference implementation and DCN ops
  character/                   per-language character files
  dict/                        per-language word dictionaries
examples/                      sample OCR images
trainer/                       recognition trainer and CRAFT trainer
unit_test/                     packaged regression/unit-test harness
scripts/                       helper scripts, e.g. Japanese char generation
custom_model.md                custom recognizer instructions
releasenotes.md                release history
requirements.txt              runtime dependencies
setup.py                      package metadata and console script
Dockerfile                    editable install in PyTorch base image
```

`MANIFEST.in` includes the runtime language resources, DBNet code, and `compile_dbnet_dcn.py` in source distributions/packages.

The package does not include large model weights directly. `Reader` downloads required `.pth` / `.pt` files into the configured model directory when missing or when an MD5 check fails.


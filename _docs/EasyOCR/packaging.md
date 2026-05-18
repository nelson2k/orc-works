# Packaging

Package metadata is in `setup.py`.

Key facts:

- package name: `easyocr`
- version: `1.7.2`
- exported package: `easyocr`
- console script: `easyocr= easyocr.cli:main`
- license: Apache-2.0
- Python dependencies are read from `requirements.txt`

Runtime dependencies listed locally:

```text
torch
torchvision>=0.5
opencv-python-headless
scipy
numpy
Pillow
scikit-image
python-bidi
PyYAML
Shapely
pyclipper
ninja
```

`MANIFEST.in` includes:

- license and README
- `easyocr/model/*`
- `easyocr/character/*`
- `easyocr/dict/*`
- `easyocr/scripts/compile_dbnet_dcn.py`
- all of `easyocr/DBNet`

The Dockerfile starts from `pytorch/pytorch`, installs OpenCV-related system libraries and git, clones EasyOCR, runs `python setup.py build_ext --inplace -j 4`, and installs the repo editable with pip.


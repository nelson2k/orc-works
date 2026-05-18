# Package Layout

Contents of `repos-folder/opencv-python/`:

```text
_build_backend/              custom PEP 517 backend shim
.github/workflows/           GitHub Actions wheel build/test/release matrices
cv2/                         source-side package placeholder and data package
docker/                      manylinux / musllinux Dockerfiles
multibuild/                  multibuild submodule path
opencv/                      OpenCV source submodule path
opencv_contrib/              optional contrib modules submodule path
opencv_extra/                test data submodule path
patches/                     build patches for OpenEXR and Qt plugins
scripts/                     build/install shell scripts and cv2 init injection
tests/                       smoke tests and sample video
setup.py                     scikit-build package and CMake orchestration
pyproject.toml               build-system requirements
find_version.py              generated cv2/version.py logic
MANIFEST.in                  source distribution include rules
LICENSE.txt                  MIT license for this packaging repo
LICENSE-3RD-PARTY.txt        bundled binary dependency licenses
```

In this checkout, `opencv`, `opencv_contrib`, and `opencv_extra` exist as directories but are not populated. A normal build initializes them with git submodule commands unless `OPENCV_PYTHON_SKIP_GIT_COMMANDS` is set.

`cv2/__init__.py` is empty in the source tree. During build, `setup.py` injects `scripts/__init__.py` into OpenCV's generated `cv2` config file so the installed package knows where to find extension modules and Linux Qt plugin/font paths.


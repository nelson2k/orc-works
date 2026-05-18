# CI And Release

GitHub Actions workflows live in `.github/workflows/`.

Main workflows:

- `build_wheels_manylinux.yml`
- `build_wheels_windows.yml`
- `build_wheels_macos.yml`
- `build_wheels_macos_m1.yml`

The wheel matrix varies by:

- platform / architecture
- Python version used to build and test
- contrib on/off
- headless on/off
- source distribution on/off where applicable
- manylinux baseline for Linux

Linux uses custom manylinux images from `quay.io/opencv-ci`. The Dockerfiles in `docker/` define image families for manylinux2014, manylinux_2_28, older manylinux1, and musllinux.

Build flow:

```text
checkout repo
set ENABLE_CONTRIB / ENABLE_HEADLESS / ENABLE_ROLLING
source scripts/build.sh or run setup.py directly
upload wheel artifacts
download wheels in test jobs
install wheel
print cv2.getBuildInformation()
run OpenCV Python tests
run pylint smoke check
upload release artifacts with twine
```

Release behavior:

- scheduled and manual workflow runs can publish rolling wheels
- prereleases upload to TestPyPI
- full GitHub releases upload to the four PyPI projects

Legacy Travis/multibuild scripts are still present and are used by shell wrappers such as `scripts/build.sh`, `scripts/install.sh`, and `travis_config.sh`.


# opencv-python - notes

Source: `repos-folder/opencv-python`. License for packaging scripts: MIT. The bundled OpenCV project is Apache-2.0, with third-party binary licenses listed in `LICENSE-3RD-PARTY.txt`.

This repository packages OpenCV's Python bindings as pre-built wheels. It is not where most `cv2` algorithms or binding generators are implemented; those live in the upstream `opencv` and optional `opencv_contrib` source trees.

The repo builds four PyPI package flavors that all import as `cv2`:

- `opencv-python`
- `opencv-contrib-python`
- `opencv-python-headless`
- `opencv-contrib-python-headless`

Main responsibilities:

- initialize/update OpenCV submodules for builds
- configure OpenCV CMake for Python wheels
- build static OpenCV binaries plus `cv2`
- collect CMake output into Python package layout
- include data files such as Haar cascades
- run wheel smoke tests and OpenCV Python tests
- publish release, prerelease, and rolling wheels

Good first files:

- `README.md` - upstream user/build docs.
- `setup.py` - package flavor selection, CMake args, and output rearrangement.
- `pyproject.toml` - build dependencies and custom backend.
- `_build_backend/backend.py` - conditional CMake build dependency.
- `find_version.py` - derives wheel version from OpenCV source plus repo tag/hash.
- `.github/workflows/` - platform build/test/release matrices.


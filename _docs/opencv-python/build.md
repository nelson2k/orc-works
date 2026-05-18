# Build

The build uses `scikit-build` to run OpenCV's CMake build from Python packaging commands.

Manual wheel build from the upstream README:

```text
git clone --recursive https://github.com/opencv/opencv-python.git
cd opencv-python
pip wheel . --verbose
```

Useful environment variables:

- `ENABLE_CONTRIB=1` - include `opencv_contrib` modules.
- `ENABLE_HEADLESS=1` - build without GUI dependencies.
- `ENABLE_JAVA=1` - enable Java client build.
- `ENABLE_ROLLING=1` - build rolling package naming/version behavior.
- `CI_BUILD=1` - emulate CI-specific behavior.
- `CMAKE_ARGS="..."` - append custom OpenCV CMake flags.
- `OPENCV_PYTHON_SKIP_GIT_COMMANDS=1` - skip submodule sync/update from `setup.py`.

Base CMake choices in `setup.py`:

- build Python 3 bindings and disable Python 2
- disable OpenCV apps, tests, perf tests, docs, freetype, and Java by default
- build static OpenCV libraries with `BUILD_SHARED_LIBS=OFF`
- install generated Python outputs into a simple `python` folder
- enable `PYTHON3_LIMITED_API`
- enable OpenEXR
- include contrib modules only when requested

`RearrangeCMakeOutput` monkey-patches scikit-build's installed-file classification. It copies only selected CMake install outputs into the final Python package, such as `cv2.*`, generated config files, `py.typed`, helper subpackages, Qt plugin/font files for CI Linux GUI wheels, and Haar cascades.

`pyproject.toml` declares build requirements. `_build_backend/backend.py` adds a `cmake>=3.5` build dependency only if the system CMake is missing or too old.


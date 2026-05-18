# Package Flavors

All four package flavors install the same import namespace:

```python
import cv2
```

Only one flavor should be installed in a Python environment at a time.

Flavor selection in `setup.py`:

| Environment | Package name |
|---|---|
| neither `ENABLE_CONTRIB` nor `ENABLE_HEADLESS` | `opencv-python` |
| `ENABLE_CONTRIB=1` | `opencv-contrib-python` |
| `ENABLE_HEADLESS=1` | `opencv-python-headless` |
| both flags | `opencv-contrib-python-headless` |
| `ENABLE_ROLLING=1` | appends `-rolling` |

`OPENCV_PYTHON_PACKAGE_NAME` can override the computed package name.

Contrib builds add:

```text
-DOPENCV_EXTRA_MODULES_PATH=<repo>/opencv_contrib/modules
```

Headless builds disable GUI/media UI dependencies:

```text
-DWITH_WIN32UI=OFF
-DWITH_QT=OFF
-DWITH_GTK=OFF
-DWITH_MSMF=OFF
-DWITH_OBSENSOR=OFF
```

On Linux headless builds also disable FFmpeg libavdevice to avoid bringing an extra `libxcb` dependency.

Package data includes the compiled `cv2` extension, `cv2/version.py`, licenses, DLLs on Windows, generated Python helper packages, type files on supported Python versions, and Haar cascade XML files under `cv2.data`.


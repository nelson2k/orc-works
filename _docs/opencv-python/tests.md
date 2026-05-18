# Tests

Local packaging smoke tests are under `tests/`.

Files:

- `test.py` - `unittest` smoke tests for `import cv2` and opening `SampleVideo_1280x720_1mb.mp4` with `cv2.VideoCapture`.
- `get_build_info.py` - prints `cv2.getBuildInformation()`.
- `SampleVideo_1280x720_1mb.mp4` - video fixture for `VideoCapture`.
- `pylintrc` - lint config used in CI.

CI does more than the local smoke tests:

- installs a built wheel
- prints build information
- runs upstream OpenCV Python tests from `opencv/modules/python/test/test.py`
- sets `OPENCV_TEST_DATA_PATH` to `opencv_extra/testdata`
- runs pylint against `opencv/samples/python/squares.py`

Because this checkout does not include populated `opencv` and `opencv_extra` submodule contents, the full upstream OpenCV test commands cannot run from local files as-is.


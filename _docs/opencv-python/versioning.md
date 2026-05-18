# Versioning

Version generation lives in `find_version.py`.

It reads OpenCV's version from:

```text
opencv/modules/core/include/opencv2/core/version.hpp
```

Then it appends a packaging identifier:

- exact git tag: `cv_major.cv_minor.cv_revision.package_revision`
- rolling build: `cv_major.cv_minor.cv_revision.YYYYMMDD`
- local/dev build: `cv_major.cv_minor.cv_revision+git_hash`

It writes:

```text
cv2/version.py
```

with:

- `opencv_version`
- `contrib`
- `headless`
- `rolling`
- `ci_build`

`setup.py` calls `find_version.py` when `.git` exists, then reads `cv2/version.py` to set the package version and build flags.

Because the local `opencv` submodule directory is not populated in this checkout, `find_version.py` cannot currently resolve the OpenCV version from local source.


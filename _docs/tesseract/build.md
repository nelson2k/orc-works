# Build

This checkout supports both Autotools and CMake.

Autotools path from `INSTALL.GIT.md`:

```text
./autogen.sh
./configure
make
sudo make install
sudo ldconfig
make training
sudo make training-install
```

CMake path:

```text
mkdir build
cd build
cmake ..
make
sudo make install
```

Core dependency:

- Leptonica is required for image loading and image-format support.

Common optional dependencies/features:

- OpenMP via `OPENMP_BUILD`.
- TIFF, libarchive, and libcurl, unless disabled with their CMake options.
- Pango, Cairo, and ICU for training tools.
- ScrollView Java dependencies for viewer/debugging support.

Notable CMake options in `CMakeLists.txt`:

- `DISABLED_LEGACY_ENGINE` - remove legacy OCR engine pieces.
- `BUILD_TRAINING_TOOLS` - build training utilities.
- `BUILD_TESTS` - build tests.
- `GRAPHICS_DISABLED` - disable ScrollView graphics.
- `OPENMP_BUILD` - build with OpenMP support.
- `FAST_FLOAT` - use float for LSTM.
- `INSTALL_CONFIGS` - install config files.

The configured language standard is C++17 by default, with conditional C++20 use in some cases.


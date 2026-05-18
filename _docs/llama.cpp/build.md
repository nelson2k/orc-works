# Building llama.cpp

The build system is CMake. Make ≥ 3.14, a C++17 compiler, and Python ≥ 3.10 for the model-conversion scripts. The pre-built tools are named `llama-*` (e.g. `llama-cli`, `llama-server`).

## Quick install (no compilation)

Several package managers ship pre-built binaries:

| Package manager | Command |
|---|---|
| Homebrew (macOS / Linux) | `brew install llama.cpp` |
| Nix | `nix profile install nixpkgs#llama-cpp` |
| Winget (Windows) | `winget install llama.cpp` |
| Docker | `docker run --rm -it ghcr.io/ggml-org/llama.cpp:full -h` |
| Releases | Download from GitHub releases — per-OS / per-arch zips |

For CPU-only or GPU builds the project also publishes Docker images: `:full`, `:full-cuda`, `:full-rocm`, `:full-musa`, `:full-cann`, `:server`, `:server-cuda`, …

## Build from source

CPU-only build:

```bash
cmake -B build
cmake --build build --config Release -j
```

That gives you `build/bin/llama-cli`, `build/bin/llama-server`, and every other tool. Add `-DBUILD_SHARED_LIBS=ON` to also produce `libllama.so` / `llama.dll`.

For repeatable parallel builds use Ninja: `cmake -B build -G Ninja`.

Debug build: `cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build`.

Install ccache for faster rebuilds.

If you want HTTPS in `llama-server`, install OpenSSL development headers before configuring (`libssl-dev` / `openssl-devel` / `openssl`).

## Backend selection (the important part)

A "backend" is a ggml device target. CPU is always present. GPU and accelerator backends are opt-in via CMake flags:

| Backend | Flag | Target hardware |
|---|---|---|
| Metal | (default on Apple Silicon) | Apple Silicon GPU |
| Accelerate (BLAS) | (default on macOS) | Apple CPU BLAS |
| CUDA | `-DGGML_CUDA=ON` | NVIDIA GPU (requires CUDA Toolkit ≥ 11.7) |
| HIP | `-DGGML_HIP=ON` | AMD GPU (ROCm) |
| MUSA | `-DGGML_MUSA=ON` | Moore Threads MTT GPU |
| Vulkan | `-DGGML_VULKAN=ON` | Any GPU with Vulkan 1.3 |
| SYCL | `-DGGML_SYCL=ON` | Intel Arc / Data Center GPUs (needs oneAPI) |
| OpenCL | `-DGGML_OPENCL=ON` | Adreno mobile GPUs (chiefly) |
| CANN | `-DGGML_CANN=ON` | Huawei Ascend NPU |
| OpenVINO | `-DGGML_OPENVINO=ON` | Intel CPU / iGPU / NPU |
| ZenDNN | `-DGGML_ZENDNN=ON` | AMD EPYC CPUs |
| zDNN | `-DGGML_ZDNN=ON` | IBM Z / LinuxONE |
| Hexagon | `-DGGML_HEXAGON=ON` | Qualcomm Snapdragon (NPU) |
| WebGPU | `-DGGML_WEBGPU=ON` | Browsers (WASM) — in progress |
| RPC | `-DGGML_RPC=ON` | Remote ggml backend over TCP |
| BLAS | `-DGGML_BLAS=ON -DGGML_BLAS_VENDOR=...` | OpenBLAS / Intel MKL / Accelerate / BLIS |

You can enable several at once. At runtime libllama loads every registered backend and uses each available device.

Quant-related and feature flags:

| Flag | Effect |
|---|---|
| `-DGGML_AVX=ON / GGML_AVX2 / GGML_AVX512 / GGML_AVX_VNNI / GGML_AMX_*` | Force-enable x86 ISAs (auto-detected by default). |
| `-DGGML_NATIVE=ON` | `-march=native` — fastest binary, only runs on the build machine. |
| `-DGGML_OPENMP=ON/OFF` | OpenMP threading. |
| `-DGGML_LTO=ON` | Link-time optimization. |
| `-DLLAMA_CURL=OFF` | Disable libcurl (needed for `-mu` HTTP model URLs). |
| `-DLLAMA_BUILD_TOOLS=OFF` | Skip building the `tools/` binaries. |
| `-DLLAMA_BUILD_EXAMPLES=OFF` | Skip building `examples/`. |
| `-DLLAMA_BUILD_TESTS=ON` | Build the test suite. |
| `-DLLAMA_BUILD_SERVER=OFF` | Skip `llama-server`. |
| `-DBUILD_SHARED_LIBS=ON` | Produce a shared `libllama` instead of static. |

## Examples

NVIDIA / CUDA:

```bash
cmake -B build -DGGML_CUDA=ON
cmake --build build --config Release -j
```

Apple Silicon (Metal is on by default):

```bash
cmake -B build
cmake --build build --config Release -j
```

Vulkan (cross-vendor GPU):

```bash
cmake -B build -DGGML_VULKAN=ON
cmake --build build --config Release -j
```

Windows ARM64 with clang:

```bash
cmake --preset arm64-windows-llvm-release -D GGML_OPENMP=OFF
cmake --build build-arm64-windows-llvm-release
```

CMake presets in `CMakePresets.json` cover several common cross-builds (`arm64-windows-llvm-release`, `arm64-linux-clang`, `arm64-apple-clang`, `x64-windows-llvm-release`, `riscv64-spacemit-linux-gnu-gcc`).

## Building only the Python conversion side

If you don't need to compile the C++ at all and only want the GGUF Python package and converters:

```bash
pip install -r requirements.txt
# or just the converter you need:
pip install -r requirements/requirements-convert_hf_to_gguf.txt
```

The `gguf-py/` package can also be installed standalone: `pip install gguf` (or `pip install -e gguf-py/` for editable use from a clone).

## Notes

- Default GPU offload is `--gpu-layers auto`, which negotiates with the device free-memory query. Pass `--gpu-layers all` to force everything on GPU, or an integer to pin a layer count.
- The runtime auto-loads any backend it finds — even if you compiled with `CUDA=ON`, on a CPU-only machine it falls back gracefully.
- HF model downloads land in the standard Hugging Face cache (`~/.cache/huggingface/hub/`), shared with other HF tools.
- `pkg-config llama` works after `cmake --install`; use `llama-config.cmake` from CMake.

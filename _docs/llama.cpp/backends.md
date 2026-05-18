# Backends

A "backend" in ggml terminology is a device-class plus the implementation of every op for that device. The CPU backend is always present; everything else is opt-in at compile time and auto-loaded at runtime.

## At a glance

| Backend | CMake flag | Target hardware |
|---|---|---|
| **CPU** | (always on) | x86 (AVX/AVX2/AVX512/AMX), ARM (NEON/SVE/SVE2/i8mm/dotprod), RISC-V (RVV/ZVFH/ZFH), PowerPC, s390x |
| **Metal** | (default on Apple) | Apple Silicon GPU |
| **BLAS** | `GGML_BLAS=ON` | OpenBLAS / Intel MKL / BLIS / Accelerate — speeds up large mat-muls (prompt eval) |
| **CUDA** | `GGML_CUDA=ON` | NVIDIA GPU |
| **HIP** | `GGML_HIP=ON` | AMD GPU (ROCm) |
| **MUSA** | `GGML_MUSA=ON` | Moore Threads MTT |
| **Vulkan** | `GGML_VULKAN=ON` | Any GPU with Vulkan 1.3 (NVIDIA, AMD, Intel, Apple via MoltenVK, Adreno) |
| **SYCL** | `GGML_SYCL=ON` | Intel Arc / Data Center GPU Max (via oneAPI DPC++) |
| **OpenCL** | `GGML_OPENCL=ON` | Qualcomm Adreno chiefly; also other OpenCL devices |
| **CANN** | `GGML_CANN=ON` | Huawei Ascend NPU |
| **OpenVINO** | `GGML_OPENVINO=ON` | Intel CPU / iGPU / NPU |
| **ZenDNN** | `GGML_ZENDNN=ON` | AMD EPYC server CPUs (oneDNN with Zen optimizations) |
| **zDNN** | `GGML_ZDNN=ON` | IBM Z / LinuxONE — Telum AI accelerator |
| **Hexagon** | `GGML_HEXAGON=ON` | Qualcomm Snapdragon Hexagon NPU |
| **VirtGPU** | `GGML_VIRTGPU=ON` | VirtGPU-passthrough VMs |
| **WebGPU** | `GGML_WEBGPU=ON` | Browsers via WASM — in progress |
| **RPC** | `GGML_RPC=ON` | Remote ggml backend over TCP (run anywhere) |

You can enable multiple at the same time and let libllama pick. On a system with both CUDA and Vulkan, build both: CUDA will be used by default (faster for NVIDIA), but Vulkan stays available as a fallback.

## CPU

`ggml/src/ggml-cpu/` contains per-ISA kernel variants. The build picks the best ISA available at runtime:

- x86: SSE3, AVX, AVX2, AVX512F, AVX512 BW/VL/VNNI, AMX (BF16 / INT8).
- ARM: NEON, fp16 vectors, BF16, i8mm, sve, sve2, dotprod.
- RISC-V: RVV (vector extension), ZVFH (FP16), ZFH (scalar FP16), ZICBOP, ZIHINTPAUSE.
- s390x: VX/VXE/VXE2.
- PowerPC: MMA / VSX.

Threading uses `ggml-threading.cpp` (a custom worker pool) or OpenMP (`-DGGML_OPENMP=ON`). On Linux NUMA-aware thread placement is available via `--numa distribute|isolate|numactl`.

## Metal

`ggml/src/ggml-metal/` — Metal shader kernels (`.metal` files compiled into a `default.metallib`). Optimized for Apple Silicon's unified-memory architecture: no host-device copies needed.

Default behavior on macOS: build automatically enables Metal. Disable with `-DGGML_METAL=OFF`.

Notable: `--gpu-layers all` works fine on M-series Macs because system RAM is the GPU's RAM.

## CUDA

`ggml/src/ggml-cuda/` — one `.cu` per op (`acc.cu`, `argmax.cu`, `binbcast.cu`, …). Requires CUDA Toolkit ≥ 11.7 and a compute capability of at least 6.0 (Pascal); 7.0+ recommended for FP16 mat-mul performance, 8.0+ (Ampere) for BF16, 8.9+ (Ada/Hopper) for FP8.

Build:

```bash
cmake -B build -DGGML_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES="86;89"
```

Multi-GPU: `-sm layer` (default) splits by layer with pipeline parallelism; `-sm row` splits each tensor row across GPUs for tensor parallelism; `-sm tensor` is full TP (experimental). `-ts` controls split ratios.

Flash attention: `-fa on` enables fused attention; usually a 1.2-2× speedup at long context.

KV-cache types `q8_0` / `q4_0` work on CUDA, saving VRAM at modest quality cost.

## HIP (AMD)

`ggml/src/ggml-hip/` mirrors the CUDA backend through hipify. Requires ROCm. Build:

```bash
cmake -B build -DGGML_HIP=ON -DAMDGPU_TARGETS="gfx1100"
```

Most CUDA features work; flash-attn quality is the main thing to verify.

## Vulkan

`ggml/src/ggml-vulkan/` — GLSL/SPIR-V shaders. Cross-vendor (NVIDIA, AMD, Intel Arc, Apple via MoltenVK, Snapdragon). Build:

```bash
cmake -B build -DGGML_VULKAN=ON
```

Often the best choice when you want one binary to work on multiple GPU vendors. Performance is competitive with native backends.

## SYCL

`ggml/src/ggml-sycl/` — Intel oneAPI DPC++. For Intel Arc and Data Center GPU Max. Setup:

```bash
source /opt/intel/oneapi/setvars.sh
cmake -B build -DGGML_SYCL=ON -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icpx
```

## CANN

`ggml/src/ggml-cann/` — Huawei's Compute Architecture for Neural Networks (Ascend NPU).

## OpenCL

`ggml/src/ggml-opencl/` — written primarily to support Adreno GPUs on Snapdragon devices, but works on any OpenCL 2.0+ device.

## OpenVINO

`ggml/src/ggml-openvino/` — Intel OpenVINO 2024+ for CPU / iGPU / NPU. Status: in progress.

## ZenDNN / zDNN

- ZenDNN — oneDNN with AMD Zen optimizations for EPYC CPU inference.
- zDNN — IBM Z/LinuxONE on-chip AI accelerator (Telum).

## Hexagon

`ggml/src/ggml-hexagon/` — Qualcomm Hexagon DSP / NPU on Snapdragon mobile chips. In progress.

## RPC

`ggml/src/ggml-rpc/` — exposes a ggml backend over TCP. The remote machine runs `rpc-server -p 50052 -H 0.0.0.0` (built from `tools/rpc/`); the client uses `--rpc host:port` and the remote device shows up as any other ggml device.

Useful to:

- Use a beefy remote GPU from a laptop.
- Pool multiple machines' GPUs for a single very-large model.
- Stage prompts on a cheap node and run generation on a GPU node.

## Backend autoload

`llama_backend_init()` calls `ggml_backend_load_all()`. That function searches:

- Statically linked backends (built with `-DGGML_*=ON`).
- Shared-library backends in the executable's directory (`ggml-cuda.so`, `ggml-vulkan.so`, …) and in `ggml/cmake/`-defined paths.
- Plugin directory under the install prefix.

If no GPU device is available the runtime gracefully falls back to CPU. `--list-devices` prints every registered device with free memory and device name.

## Device selection at runtime

```bash
llama-cli --list-devices
# CPU0     - CPU - ... - 32 GiB
# CUDA0    - GPU - NVIDIA RTX 4070 - 12 GiB
# Vulkan0  - GPU - NVIDIA RTX 4070 - 12 GiB

llama-cli -m model.gguf -dev CUDA0      # pick a specific device
llama-cli -m model.gguf -dev CUDA0,CUDA1 # use two
llama-cli -m model.gguf -dev none       # CPU only
```

`-ngl auto` (default) negotiates with the device's free-memory query; `-ngl all` forces full offload; an integer pins a layer count. `--fit on` adjusts unset args to fit within target margins (`--fit-target` MiB per device).

## Op coverage

`docs/ops.md` is auto-generated and lists every ggml op × every backend, marking which combinations are implemented. When porting a new model architecture, this is the matrix to check first.

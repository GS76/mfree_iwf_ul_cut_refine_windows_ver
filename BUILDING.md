# Building (CMake)

This project supports a cross-platform CMake-based build. The legacy/manual `g++` workflow is considered a fallback.

## Requirements
- **CMake**: ≥ 3.16 (recommended: latest stable)
- **C++ compiler** (C++17):
  - Linux: GCC ≥ 9 or Clang ≥ 10
  - macOS: AppleClang ≥ 12 (or Clang ≥ 10 via Homebrew)
  - Windows: MSVC 2019+ or MinGW-w64 GCC ≥ 9
- **Build tool**:
  - Recommended: Ninja
  - Alternatives: Makefiles (Linux/macOS), Visual Studio generators (Windows)

Dependencies:
- **OpenMP** is required for the main simulation target.
- **GLM** is pulled automatically via CMake (FetchContent) unless you disable it.

## Quick Start (Scripts)
### Windows
```bat
build.bat --config Release --clean --parallel 8
```

### Linux/macOS
```bash
./build.sh --config Release --clean --parallel 8
```

Both scripts support:
- `--build-dir <dir>`
- `--config Debug|Release`
- `--prefix <install-prefix>`
- `--generator <CMake generator>`
- `--parallel <N>`
- `--package`

## Manual CMake (All Platforms)
Single-config generator (Ninja/Makefiles):
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 8
ctest --test-dir build --output-on-failure
cmake --install build
```

Multi-config generator (Visual Studio):
```powershell
cmake -S . -B build-vs -G "Visual Studio 17 2022"
cmake --build build-vs --config Release --parallel 8
ctest --test-dir build-vs -C Release --output-on-failure
cmake --install build-vs --config Release
```

## Installing CMake and Adding it to PATH
### Windows
- Install from https://cmake.org/download/ and select “Add CMake to the system PATH”.
- Verify:
  ```powershell
  cmake --version
  ```

### Linux
- Ubuntu/Debian:
  ```bash
  sudo apt-get update
  sudo apt-get install -y cmake ninja-build g++
  ```
- Verify:
  ```bash
  cmake --version
  ninja --version
  g++ --version
  ```

### macOS
- Homebrew:
  ```bash
  brew install cmake ninja llvm
  ```
- Verify:
  ```bash
  cmake --version
  ninja --version
  clang --version
  ```

## OpenMP Notes (Toolchain-specific)
- **Linux GCC**: OpenMP is typically available by default (`-fopenmp`).
- **Linux Clang**: may require installing `libomp` and using a supported runtime.
- **macOS AppleClang**: OpenMP may require `libomp` (Homebrew) and explicit configuration.
- **Windows MinGW-w64 GCC**: OpenMP works via `-fopenmp` if the runtime is available.
- **Windows MSVC**: OpenMP is supported by MSVC, but behavior/flags differ (CMake handles this via `find_package(OpenMP)`).

## When a Full Rebuild is Required
Reconfigure and rebuild from scratch (delete build directory) if any of these conditions occur:
- Source layout or target graph changes:
  - adding/removing `.cpp` files
  - moving files between `src/`, `src/benchmarks/`, `src/tests/`
- Build configuration changes:
  - edits to `CMakeLists.txt`
  - switching generator (Ninja ↔ Visual Studio ↔ Makefiles)
  - switching toolchain (GCC ↔ Clang ↔ MSVC) or upgrading major versions
- Dependency changes:
  - updating GLM pin/version (FetchContent) or switching to `find_package(glm)`
  - changing OpenMP runtime availability
- Build artifact corruption:
  - CMake cache mismatches, “CMAKE_CXX_COMPILER not set”, stale object files, or linker errors caused by mixed configurations

Incremental builds (no full rebuild) are usually fine when:
- only `.cpp` files change and you keep the same generator/toolchain/config
- only local code changes occur without changing dependencies or global flags

## How to Verify a Successful Rebuild
- Configure completes without errors.
- Build produces:
  - `mfree_iwf` executable
  - (optional) `validate_omp` benchmark
  - (optional) unit test executables
- Tests pass:
  ```bash
  ctest --test-dir build --output-on-failure
  ```
- Smoke test passes:
  - `ctest -R smoke_model_1`
- The main binary can start:
  ```bash
  ./mfree_iwf -m 1 --smoke
  ```

## Build Optimization Recommendations
- Prefer Ninja for faster incremental builds.
- Enable parallel builds (`--parallel`).
- Use a compiler cache where available:
  - Linux/macOS: `ccache`
  - Windows: `sccache` (or compiler-native caching if available)
- Keep separate build directories per toolchain and configuration (e.g., `build-gcc-release`, `build-msvc-debug`).

## Reproducibility Procedure
1. Delete the build directory.
2. Configure and build from scratch.
3. Run `ctest`.
4. Record tool versions (`cmake --version`, compiler version).

## Compatibility Matrix
This matrix tracks known-good configure/build/test runs.

| OS | Compiler | Version | Generator | Status |
|---|---|---:|---|---|
| Windows | GCC (MinGW) | 14.1 | Ninja (bootstrap) | Verified |
| Windows | MSVC | 2019+ | Visual Studio | Not yet verified |
| Linux | GCC | 9+ | Ninja/Makefiles | Not yet verified |
| Linux | Clang | 10+ | Ninja/Makefiles | Not yet verified |
| macOS | AppleClang | 12+ | Ninja/Makefiles | Not yet verified |

## Measuring Build-Time Improvements
Recommended comparison:
1. Legacy build (single-step `g++` compile+link).
2. CMake build with Ninja, incremental rebuild after touching one `.cpp`.

Record:
- Configure time
- Clean build time
- Incremental build time (touch one `.cpp` and rebuild)

### Local reference numbers (this workspace)
- CMake configure time: ~33.3s (Windows, MinGW GCC 14.1, Ninja)
- Clean build + install wall time: ~40.5s (same environment)

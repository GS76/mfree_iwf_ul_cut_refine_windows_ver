# Building on Windows

This project has been ported to support Windows.

## Dependencies

- **CMake**: ≥ 3.16 (recommended: latest stable)
- **C++ Compiler**: GCC (MinGW-w64) or MSVC with C++17 support.
- **Build tool**: Ninja (recommended) or a Makefiles/Visual Studio generator toolchain.
- **GLM**: handled automatically by the CMake build (FetchContent) by default.
- **OpenMP**: required for the main simulation target.

## Recommended: CMake build

Use the automated script:

```powershell
.\build.bat --config Release --clean --parallel 8
```

Manual CMake (example with Ninja):

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 8
ctest --test-dir build --output-on-failure
```

## Running

```bash
.\mfree_iwf.exe -m 1
```

## Legacy fallback: manual G++ build (MinGW)
If you need a quick one-off build without CMake, this repository previously used:

```powershell
$files = Get-ChildItem -Recurse src -Filter *.cpp | Where-Object { $_.FullName -notmatch '\\src\\tests\\' -and $_.Name -ne 'test_omp_scaling.cpp' }
g++ -o mfree_iwf.exe $files.FullName -I src -I src/benchmarks -I deps/glm-0.9.9.8 -std=c++17 -fopenmp -D_USE_MATH_DEFINES
```

## Documentation
For detailed CMake installation instructions, PATH verification, toolchain notes (MinGW/MSVC), and troubleshooting, see:
- [BUILDING.md](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/BUILDING.md)

## Notes for Windows Compatibility

- Replaced POSIX headers (`<sys/time.h>`, `<unistd.h>`, etc.) with C++ standard headers (`<chrono>`, `<filesystem>`).
- Replaced `gettimeofday` with `std::chrono::high_resolution_clock`.
- Replaced `mkdir` and `system("rm ...")` with `std::filesystem::create_directory` and `std::filesystem::remove`.
- Added `_USE_MATH_DEFINES` for `M_PI` compatibility.
- Added `deps/glm-0.9.9.8` for GLM dependency.

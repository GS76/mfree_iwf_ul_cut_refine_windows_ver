# Building on Windows

This project has been ported to support Windows.

## Dependencies

- **C++ Compiler**: GCC (MinGW) or MSVC with C++17 support.
- **GLM**: OpenGL Mathematics library (header-only). Included in `deps/glm-0.9.9.8` for convenience.
- **OpenMP**: (Optional) For parallel processing.

## Building with G++ (MinGW)

You can build the project using the following command from the project root:

```powershell
$files = Get-ChildItem -Recurse src -Filter *.cpp
g++ -o mfree_iwf.exe $files.FullName -I src -I src/benchmarks -I deps/glm-0.9.9.8 -std=c++17 -fopenmp -D_USE_MATH_DEFINES
```

## Building with CMake

A `CMakeLists.txt` file is provided. If you have CMake installed:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Running

```bash
./mfree_iwf.exe -m 1
```

## Changes for Windows Compatibility

- Replaced POSIX headers (`<sys/time.h>`, `<unistd.h>`, etc.) with C++ standard headers (`<chrono>`, `<filesystem>`).
- Replaced `gettimeofday` with `std::chrono::high_resolution_clock`.
- Replaced `mkdir` and `system("rm ...")` with `std::filesystem::create_directory` and `std::filesystem::remove`.
- Added `_USE_MATH_DEFINES` for `M_PI` compatibility.
- Added `deps/glm-0.9.9.8` for GLM dependency.

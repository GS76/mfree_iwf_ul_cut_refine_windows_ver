# OpenMP Build & Compliance Report

## Scope
- Build verification for OpenMP-enabled targets via CMake.
- Runtime verification using the OpenMP benchmark executable (`validate_omp`) across thread counts `1, 2, 4, max`.
- Thread-safety review of key `#pragma omp` regions in the solver.

## Environment (This Run)
- OS: Windows
- Build system: CMake + Ninja (generator in `build/`)
- C++ compiler (GCC/MinGW): GCC 14.1.0
- MSVC: Not available in PATH (`cl` not found), so MSVC verification was skipped.

## OpenMP Build Verification

### CMake OpenMP Detection
- `build/CMakeCache.txt` reports:
  - `OpenMP_CXX_FLAGS=-fopenmp`
  - `OpenMP_CXX_SPEC_DATE=201511` (OpenMP 4.5)

### Effective Compile/Link Flags (GCC)
- `validate_omp` build shows `-fopenmp` on compile and link:
  - Artifact: `results/gcc_validate_omp_build_verbose.txt`
- `mfree_iwf` build shows `-fopenmp` on compile and link for core and main translation units:
  - Artifact: `results/gcc_mfree_iwf_build_verbose.txt`

### Compilation Warnings
- No compiler warnings were observed in the captured verbose build logs above.

## OpenMP Test Execution

### CTest Suite (Repository Tests)
- Command: `ctest --test-dir build --output-on-failure`
- Result: PASS (2/2)
  - `test_property_interpolation`: PASS
  - `smoke_model_1`: PASS

### OpenMP Benchmark (`validate_omp`)
- Executable: `build/validate_omp.exe`
- Runs executed with `OMP_NUM_THREADS` set to `1, 2, 4, 12` (max logical processors in this environment).
- Exit status: SUCCESS for all runs.
- Output artifacts:
  - `results/gcc_validate_omp_threads_1.txt`
  - `results/gcc_validate_omp_threads_2.txt`
  - `results/gcc_validate_omp_threads_4.txt`
  - `results/gcc_validate_omp_threads_12.txt`

#### Observations / Warnings
- Some runs print:
  - `libgomp: Affinity not supported on this configuration`
  - This is a runtime/affinity capability warning from the OpenMP runtime; it did not prevent correct execution.
- When `OMP_NUM_THREADS` is set below the system processor count, the benchmark’s “System Topology Verification” prints a mismatch warning because it compares `omp_get_max_threads()` against `omp_get_num_procs()`. This is expected under an explicit `OMP_NUM_THREADS` cap.
- At higher thread counts the benchmark sometimes reports “Core Overlap Detected” in its affinity check (consistent with the affinity warning above).

#### Representative Scaling Result (OMP_NUM_THREADS = max = 12)
From `results/gcc_validate_omp_threads_12.txt`:
- Compute-bound speedup: ~5.20x at 12 threads (efficiency ~43%)
- Memory-bound bandwidth: ~18–23 GB/s, limited scaling beyond 2 threads (expected for memory-bound kernels)

## Thread-Safety Review (Key Parallel Regions)

### Header Usage
OpenMP headers are included where pragmas are used:
- Examples: `src/thermal.cpp`, `src/leap_frog.cpp`, `src/plasticity.cpp`, `src/correctors.cpp`, `src/cont_mech.cpp`, `src/contact.cpp`, `src/derivatives.cpp`.

### Parallel Loop Patterns
Most solver kernels use `#pragma omp parallel for` over particle indices and only write to per-particle state for the current index `i`:
- This is data-parallel and thread-safe under the assumption that neighbor lists are read-only during the kernel execution.

### Notable Synchronization
- `src/plasticity.cpp` uses `#pragma omp critical` for debug printing on solver failure and `#pragma omp atomic write` for a shared failure flag.
- `src/thermal.cpp` uses OpenMP `reduction(max: ...)` to compute global maxima used for cooling-rate limiting.

### Potential Caveats (Documented)
- `thermal` stores “last observed” ramp/rate values in mutable members for reporting. This is safe under the current execution model (thermal solve called once per step on the main thread), but it would not be safe if `thermal::conduction()` were ever called concurrently from multiple threads.

## Compliance Status Summary
- GCC/MinGW OpenMP: PASS
  - OpenMP flags present (`-fopenmp`)
  - OpenMP headers present (`<omp.h>`)
  - CTest suite passes
  - OpenMP benchmark runs across thread counts without errors
- MSVC OpenMP: NOT AVAILABLE (skipped)
  - Reason: `cl` not found in this environment; requires a Visual Studio toolchain shell.


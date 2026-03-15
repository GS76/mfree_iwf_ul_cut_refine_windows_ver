# OpenMP Build & Test Verification Spec

## Why
The solver relies on OpenMP for parallel execution, so the build must consistently enable OpenMP and the OpenMP-specific executables/tests must run correctly across supported toolchains.

## What Changes
- Verify the project compiles with OpenMP enabled using the repository’s supported build workflow (CMake).
- Verify OpenMP headers and pragmas are used correctly and do not introduce thread-safety issues in parallel regions.
- Run the OpenMP-specific benchmark/test executable(s) across multiple thread counts and capture pass/fail status and any warnings.
- Document the OpenMP compliance status: compiler flags, linkage, runtime behavior, and test results.

## Impact
- Affected specs: build verification, OpenMP correctness, performance benchmark validation
- Affected code:
  - Build config: `CMakeLists.txt`
  - OpenMP benchmark: `src/benchmarks/test_omp_scaling.cpp`
  - Core parallel loops: `src/thermal.cpp`, `src/leap_frog.cpp`, `src/plasticity.cpp`, `src/adaptivity.cpp` (and any others using `#pragma omp`)
  - Test harness: `CTest` configuration in `CMakeLists.txt`
  - Documentation output: new report under `results/` or `docs/` (as agreed by tasks)

## ADDED Requirements

### Requirement: OpenMP-enabled compilation
The system SHALL compile successfully with OpenMP support enabled for the active compiler/toolchain.

#### Scenario: GCC/Clang build uses `-fopenmp`
- **WHEN** the user configures and builds with GCC or Clang
- **THEN** the compile and link steps include OpenMP support (e.g., `-fopenmp` or toolchain-equivalent)

#### Scenario: MSVC build uses OpenMP support
- **WHEN** the user configures and builds with MSVC
- **THEN** the compile and link steps include MSVC OpenMP support (e.g., `/openmp` or toolchain-equivalent)

### Requirement: OpenMP test execution across thread counts
The system SHALL run OpenMP-specific executable(s) across multiple thread counts without errors.

#### Scenario: Run with multiple OMP thread counts
- **WHEN** OpenMP tests are executed with `OMP_NUM_THREADS` set to a set of values (e.g., 1, 2, 4, max)
- **THEN** each run completes successfully and produces expected outputs without crashes or invalid results

### Requirement: Dual-toolchain verification (GCC + MSVC)
The system SHALL support verifying OpenMP builds and tests using both GCC (MinGW) and MSVC toolchains when available in the environment.

#### Scenario: Run verification on both toolchains
- **WHEN** the user selects GCC and MSVC verification
- **THEN** the report includes results for both toolchains or clearly states which one could not be executed and why

### Requirement: OpenMP compliance report
The system SHALL produce a report summarizing:
- Detected compiler/toolchain and OpenMP runtime
- OpenMP compile/link flags used
- Any compilation warnings related to OpenMP code paths
- Test results and thread-count matrix
- Noted thread-safety review findings for key parallel regions

## MODIFIED Requirements
None.

## REMOVED Requirements
None.

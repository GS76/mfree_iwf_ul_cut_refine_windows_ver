# Tasks
- [x] Task 1: Audit the current manual g++ workflow and document gaps.
  - [x] Identify current compile/link flags, include paths, and excluded sources.
  - [x] Identify executables produced today and their entry points.
  - [x] Identify dependency sources (GLM, OpenMP runtime) and how they are discovered.

- [x] Task 2: Design the CMake target graph and source grouping.
  - [x] Define main executable target mapping to `src/refine_cut_main.cpp`.
  - [x] Define benchmark targets (including `src/benchmarks/test_omp_scaling.cpp`) to avoid `main()` conflicts.
  - [x] Decide how to treat `src/tests/` (CTest integration, optional build, or separate target).

- [x] Task 3: Implement root CMakeLists.txt with cross-platform toolchain support.
  - [x] Enforce minimum CMake version 3.16.
  - [x] Configure C++ standard (C++17) and per-config flags (Debug/Release).
  - [x] Configure OpenMP detection/linking (required for main target).
  - [x] Configure GLM dependency using `FetchContent` or `find_package` fallback.
  - [x] Add platform-specific linking and runtime settings (MSVC runtime selection; Windows definitions if needed).

- [x] Task 4: Add cross-platform build automation scripts.
  - [x] Create `build.sh`: configure/build/install/package with configurable generator and install prefix.
  - [x] Create `build.bat`: configure/build/install/package with configurable generator and install prefix.
  - [x] Support parallel builds and clean-build mode in both scripts.

- [x] Task 5: Update documentation for build/installation across Windows, Linux, macOS.
  - [x] Document installing CMake ≥ 3.16 and adding it to PATH.
  - [x] Document toolchain options (GCC/Clang/MSVC) and OpenMP enablement.
  - [x] Document dependency management (GLM) and troubleshooting common failures.

- [x] Task 6: Add testing and verification hooks.
  - [x] Integrate CTest and enable running existing unit tests (if present/enabled).
  - [x] Add a smoke test target/command that runs `-m 1` for a short duration (non-interactive).

- [x] Task 7: Validate reproducibility and measure build-time improvements.
  - [x] Perform clean builds and incremental builds; record configure/build times.
  - [x] Produce a compatibility matrix for GCC 9+, Clang 10+, MSVC 2019+ across Windows/Linux/macOS.
  - [x] Confirm generated binaries and targets remain compatible with downstream usage expectations.

# Task Dependencies
- Task 2 depends on Task 1
- Task 3 depends on Task 2
- Task 4 depends on Task 3
- Task 5 depends on Task 3
- Task 6 depends on Task 3
- Task 7 depends on Task 4 and Task 6

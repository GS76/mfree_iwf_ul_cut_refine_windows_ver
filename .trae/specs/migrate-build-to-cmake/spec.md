# CMake Build Migration Spec

## Why
The project currently relies on ad-hoc/manual `g++` compilation, which is brittle across platforms and makes it hard to reproduce builds, manage dependencies, and run consistent CI-style verification.

## What Changes
- Introduce a modern, cross-platform CMake-based build that replaces the legacy manual `g++` workflow.
- Define explicit build targets for the main simulation executable and benchmark/test executables.
- Standardize build types and compiler flags across GCC/Clang/MSVC.
- Integrate third-party dependencies (notably GLM) via CMake dependency mechanisms.
- Add cross-platform build scripts to automate configure/build/install/package.
- Update documentation with installation, PATH setup, and troubleshooting guides.
- Add build reproducibility and compatibility reporting (compiler/platform matrix) and basic build-time benchmarking.

## Impact
- Affected specs: build/release workflow, dependency management, test execution, multi-platform support
- Affected code:
  - Root build configuration: `CMakeLists.txt`
  - Source layout used for targets: `src/`, `src/benchmarks/`, `src/tests/`
  - Documentation: `README_WINDOWS.md` (and/or project README files)
  - New scripts: `build.sh`, `build.bat`

## ADDED Requirements

### Requirement: Cross-platform CMake build
The system SHALL provide a CMake build configuration that can build the project on Windows, Linux, and macOS using supported toolchains.

#### Scenario: Configure and build (Windows, MinGW)
- **WHEN** the user runs the Windows build script or invokes CMake with a MinGW generator
- **THEN** CMake configures successfully, finds a C++ compiler, and produces build files
- **AND** `cmake --build` produces the main executable and any enabled auxiliary targets

#### Scenario: Configure and build (Windows, MSVC)
- **WHEN** the user configures with a Visual Studio generator
- **THEN** CMake configures successfully and selects an MSVC runtime according to the build type
- **AND** `cmake --build` produces the main executable and any enabled auxiliary targets

#### Scenario: Configure and build (Linux/macOS)
- **WHEN** the user configures with Ninja or Makefiles
- **THEN** CMake configures successfully and produces build files
- **AND** `cmake --build` produces the main executable and any enabled auxiliary targets

### Requirement: Build types and flags
The system SHALL support Debug and Release builds with compiler-appropriate flags.

#### Scenario: Debug build flags
- **WHEN** `CMAKE_BUILD_TYPE=Debug` (single-config generators) or the Debug config is selected (multi-config generators)
- **THEN** compilation uses debug symbols and defines `DEBUG`
- **AND** optimization is disabled (`-O0` on GCC/Clang; MSVC equivalent)

#### Scenario: Release build flags
- **WHEN** `CMAKE_BUILD_TYPE=Release` (single-config generators) or the Release config is selected (multi-config generators)
- **THEN** compilation uses high optimization (`-O3` on GCC/Clang; MSVC equivalent)
- **AND** defines `NDEBUG`

### Requirement: OpenMP integration
The system SHALL automatically detect and link OpenMP when available, and fail with a clear error when OpenMP is required but not found.

#### Scenario: OpenMP found
- **WHEN** OpenMP is available for the chosen compiler
- **THEN** the main executable target is compiled and linked with OpenMP enabled

#### Scenario: OpenMP not found
- **WHEN** OpenMP is not available
- **THEN** the configuration step reports actionable guidance for enabling OpenMP on the platform/toolchain

### Requirement: Dependency management (GLM)
The system SHALL integrate GLM via `find_package()` or `FetchContent` with a pinned version and without requiring manual user installation.

#### Scenario: Clean configure
- **WHEN** the user runs a clean configure on a machine without GLM installed
- **THEN** the build system retrieves or locates GLM automatically and configures successfully

### Requirement: Target structure and entry points
The system SHALL define explicit targets and avoid multiple `main()` compilation conflicts.

#### Scenario: Main executable
- **WHEN** the main target is built
- **THEN** it includes `src/refine_cut_main.cpp` and the required simulation sources

#### Scenario: Benchmark executables
- **WHEN** benchmark targets are built
- **THEN** files with their own `main()` (e.g., `src/benchmarks/test_omp_scaling.cpp`) are built only into their dedicated executable target(s)
- **AND** are excluded from the main executable target sources

### Requirement: Cross-platform build scripts
The system SHALL provide `build.sh` and `build.bat` that automate configure, build, install, and optional packaging.

#### Scenario: Custom install prefix
- **WHEN** the user specifies an install prefix
- **THEN** install artifacts are placed under that prefix and can be packaged

#### Scenario: Parallel build
- **WHEN** the user requests a parallel build
- **THEN** the script passes an appropriate parallelism value to the underlying build tool

### Requirement: Documentation and troubleshooting
The system SHALL document CMake installation and PATH verification procedures for Windows, Linux, and macOS, including common failure modes.

#### Scenario: PATH verification
- **WHEN** a user follows the documentation
- **THEN** they can verify `cmake --version` and diagnose PATH issues with clear steps

### Requirement: Reproducibility and compatibility matrix
The system SHALL define a reproducible clean-build process and a compatibility matrix for supported platforms/compilers.

#### Scenario: Clean build reproducibility
- **WHEN** a user deletes the build directory and performs a rebuild
- **THEN** the build succeeds and produces equivalent artifacts (same target set and version metadata)

## MODIFIED Requirements

### Requirement: Legacy manual compilation workflow
The legacy `g++` manual build SHALL be treated as a fallback only, and documentation SHALL recommend the CMake workflow as the primary supported path.

## REMOVED Requirements

### Requirement: Manual build as primary workflow
**Reason**: Manual compilation is not reliably portable and is hard to maintain as dependencies and targets grow.
**Migration**: Users switch to CMake scripts (`build.sh` / `build.bat`) or direct CMake commands documented in the updated READMEs.


# Plan: Recompilation and Build Process

## Summary
Perform a full recompilation of the project using the CMake build system to incorporate recent changes to `src/refine_cut_main.cpp` (exception handling) and ensure all source modifications are reflected in the final executable. This process includes cleaning old build artifacts, configuring the project for MinGW/GCC, and compiling the target.

## 1. Explore & Analyze
- **Current State**: The source code has been modified to include robust floating-point exception handling. The existing `mfree_iwf.exe` is outdated.
- **Build System**: The project uses `CMakeLists.txt` and requires a C++17 compliant compiler with OpenMP support.
- **Constraints**: 
    - Must link against `OpenMP`.
    - Must define `_USE_MATH_DEFINES` for M_PI on Windows.
    - Must NOT use `-ffast-math`.

## 2. Proposed Steps

### Step 1: Clean Build Environment
- Remove the existing `build` directory (if present) to ensure a clean configuration state.
- Remove old executables (`mfree_iwf.exe`, `mfree_iwf_ul_cut_refine_windows_ver.exe`) to prevent confusion.

### Step 2: Configure with CMake
- Create a new `build` directory.
- Run `cmake` to generate MinGW Makefiles.
    - Generator: `"MinGW Makefiles"`
    - Source: `..` (Root directory)
    - Options: `-DCMAKE_BUILD_TYPE=Release` (for optimization)

### Step 3: Compile
- Run `cmake --build .` inside the build directory.
- Verify that the executable is created successfully.

### Step 4: Verification
- Run the newly compiled executable with a basic test case (e.g., `-m 1`) to ensure it starts without immediate errors.
- Verify that the floating-point exception handler is active (implied by successful startup and no immediate crash on initialization).

## 3. Assumptions
- The user has `cmake` and `mingw32-make` (or `make`) installed and in their system PATH.
- The `deps/glm` library is available and correctly located by CMake.

## 4. Verification Criteria
- **Success**: The build completes with "Build target mfree_iwf_ul_cut_refine_windows_ver" and an executable is produced.
- **Failure**: CMake configuration errors (missing dependencies) or compilation errors (syntax errors).

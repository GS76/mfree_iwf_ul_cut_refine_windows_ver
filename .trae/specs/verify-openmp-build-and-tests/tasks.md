# Tasks
- [x] Task 1: Verify OpenMP build flags and linkage.
  - [x] Configure and build with CMake; confirm OpenMP is found and linked.
  - [x] Capture the effective OpenMP compile/link flags for the main target and benchmark target.
  - [x] Record compilation warnings (if any) related to OpenMP code paths.

- [x] Task 2: Run OpenMP-specific executables across thread counts.
  - [x] Build and run the OpenMP benchmark/executable(s) with `OMP_NUM_THREADS=1,2,4` and a “max” value.
  - [x] Collect outputs and exit codes; flag regressions or runtime errors.

- [x] Task 3: Review thread safety of key parallel regions.
  - [x] Identify shared-state writes inside `#pragma omp parallel` regions.
  - [x] Confirm reductions/atomics/critical sections are used where needed.
  - [x] Document any hazards and fix if required.

- [x] Task 4: Produce an OpenMP compliance report.
  - [x] Write a short report file (markdown) in repo root summarizing toolchain, flags, tests, and findings.
  - [x] Include the thread-count matrix and pass/fail results.

# Task Dependencies
- Task 2 depends on Task 1
- Task 4 depends on Tasks 1–3

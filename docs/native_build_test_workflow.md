# Native Build and Test Workflow (Quick Reference)

This document summarizes the native (non-Docker) CMake workflow for local development.

## Prerequisites

- Run commands from the repository root.
- Use the canonical build directory: `build`.
- Ensure CMake and a C++ toolchain are available.

## 1) Configure

```powershell
cmake -S . -B build
```

Expected result: CMake reports `Configuring done` and `Generating done`, and writes files to `build/`.

## 2) Build (Release)

```powershell
cmake --build build --config Release
```

Expected result: `mfree_iwf.exe` and `mfree_iwf_validate.exe` are generated under `build/Release`.

## 3) Run Test Suite (Release)

```powershell
ctest -C Release --test-dir build --output-on-failure
```

Expected result: all tests pass (for example, `100% tests passed`).

## 4) Run Executables

Primary solver:

```powershell
.\build\Release\mfree_iwf.exe
```

Validation executable:

```powershell
.\build\Release\mfree_iwf_validate.exe
```

## Common Validation Mode (Preprocess + Geometry Check)

For a fast geometry sanity check before longer runs:

```powershell
$env:MFREE_PREPROCESS_ONLY = "1"
$env:MFREE_GEOM_VALIDATE = "1"
.\build\Release\mfree_iwf.exe
```

Optional cleanup in current shell:

```powershell
Remove-Item Env:\MFREE_PREPROCESS_ONLY -ErrorAction SilentlyContinue
Remove-Item Env:\MFREE_GEOM_VALIDATE -ErrorAction SilentlyContinue
```

## Troubleshooting

- If configure fails, re-run `cmake -S . -B build` and inspect the first error.
- If build fails, re-run `cmake --build build --config Release` and fix the first compile/link error.
- If tests fail, run `ctest -C Release --test-dir build --output-on-failure` and investigate the first failing test.
- Avoid alternate build directories (`build2`, `cmake-build-*`) for consistency with project workflow.

# mfree_iwf-ul_cut-refine

This repository contains the C++ source code for the paper:

- **Meshfree Simulation of Metal Cutting: An Updated Lagrangian Approach with Dynamic Refinement** (International Journal of Mechanical Sciences): https://www.sciencedirect.com/science/article/pii/S0020740319317023

It is a high-performance **meshfree metal-cutting simulation** (Updated Lagrangian RKPM/SPH-style) with thermo-mechanical coupling, plasticity, contact, and dynamic refinement.

This repository is **not** an “AI agent” project (no LLM integration, no HTTP API service, no database). Where the documentation template below requests “API endpoints” or “database schema”, those sections are explicitly marked **Not applicable** for this codebase.

## Table of Contents

- [1. Project Overview](#1-project-overview)
- [2. Architecture](#2-architecture)
- [3. Feature Inventory](#3-feature-inventory)
- [4. Interfaces](#4-interfaces)
- [5. Configuration](#5-configuration)
- [6. Data Formats](#6-data-formats)
- [7. Dependencies](#7-dependencies)
- [8. Build & Install](#8-build--install)
- [9. Run & Deploy](#9-run--deploy)
- [10. Testing](#10-testing)
- [11. Maintenance & Extension](#11-maintenance--extension)
- [12. Troubleshooting](#12-troubleshooting)
- [13. License & Credits](#13-license--credits)

## 1. Project Overview

The purpose of this package is efficient, accurate simulation of orthogonal metal cutting using meshfree discretization with refinement tailored to the Updated Lagrangian frame.

Selected modeling/stabilization features include:

- Modified Johnson–Cook constitutive model for Ti–6Al–4V (Sima & Özel 2010).
- Stabilization following Gray–Monaghan–Swift (SPH elastic dynamics).
- Thermal coupling via heat conduction using Particle Strength Exchange (PSE) or Brookshaw SPH.
- Dynamic refinement via particle splitting/merging patterns.

### Example figures (from the original publication)

Density error introduced by particle splitting:

![density](https://raw.githubusercontent.com/mroethli/mfree_iwf-ul-cut-refine/master/img/density.png)

Cutting geometry:

![cutting_sketch](https://raw.githubusercontent.com/mroethli/mfree_iwf-ul-cut-refine/master/img/cutting_sketch2.png)

Chip morphology vs resolution:

![superimposed](https://raw.githubusercontent.com/mroethli/mfree_iwf-ul-cut-refine/master/img/superimposed.png)

Comparison of refinement strategies:

![all_cuts](https://raw.githubusercontent.com/mroethli/mfree_iwf-ul-cut-refine/master/img/all_cuts.png)

### Benchmarking overview (paper reference)

| Model | Resolution at start | Resolution at end | Runtime |
| ---: | ---: | ---: | ---: |
| single low-resolution | ~6,200 | ~6,200 | 59 |
| dynamic refinement | ~6,800 | ~11,900 | 182 |
| a priori refinement | ~15,800 | ~15,800 | 362 |
| single high-resolution | ~24,400 | ~24,400 | 540 |

Runtimes reported in CPU minutes (single core), legacy VTK output viewable in ParaView.

## 2. Architecture

High-level flow:

1. Build a `body` (particles + tool + physical parameters) either from built-in benchmark models or from JSON config.
2. Run explicit time integration (2nd-order leapfrog) for coupled mechanics/thermal/plasticity/contact.
3. Log results to VTK (`.vtk`) and text/CSV outputs (`trace.txt`, `*_forces`, `cooldown_*.{csv,txt}`).

Primary entry points:

- Main simulation executable: `src/refine_cut_main.cpp`
- Setup/visualization helper: `src/tools/view_setup.cpp`

Key internal data model:

- `particle`: state container for position, velocity, density, smoothing length, stresses, temperature, neighbor list slots.
- `body`: owns particle array and modules (tool, thermal, plasticity, adaptivity), and builds neighbor lists each step.

Core time stepping:

- `leap_frog::step` performs neighbor update → predictor → RHS assembly → corrector → plasticity → boundary conditions → adaptivity.

Further reading:

- [TECHNICAL_OVERVIEW.md](TECHNICAL_OVERVIEW.md)

## 3. Feature Inventory

### 3.1 Executables and responsibilities

| Target | Purpose | Primary source |
|---|---|---|
| `mfree_iwf` | Main simulator (benchmark or JSON-config driven) | `src/refine_cut_main.cpp` |
| `view_setup` | Writes a single setup frame and outline VTK | `src/tools/view_setup.cpp` |
| `validate_omp` | OpenMP scaling/diagnostic benchmark | `src/benchmarks/test_omp_scaling.cpp` |
| `test_json_unicode` | JSON Unicode escape regression test | `src/tests/test_json_unicode.cpp` |
| `test_property_interpolation` | Table/linear property interpolation tests | `src/tests/test_property_interpolation.cpp` |

### 3.2 Modules (by responsibility)

| Module | What it does | Key files |
|---|---|---|
| State containers | Particle/body storage, module wiring | `src/particle.{h,cpp}`, `src/body.{h,cpp}` |
| Time integration | Explicit leapfrog predictor/corrector | `src/leap_frog.{h,cpp}` |
| Neighbor search | Spatial hashing + Verlet lists | `src/grid.{h,cpp}` |
| Kernel/shape functions | Cubic spline kernel; SPH or CSPM corrected gradients | `src/kernel.{h,cpp}`, `src/precomp_shape_functions.{h,cpp}` |
| Derivative operators | Velocity gradient, stress divergence | `src/derivatives.{h,cpp}` |
| Stabilizations/correctors | Artificial viscosity, Monaghan stress correction, XSPH | `src/correctors.{h,cpp}` |
| Continuum mechanics | Continuity, momentum, advection, mechanical BC enforcement | `src/cont_mech.{h,cpp}` |
| Material/EOS | Pressure EOS and stress-rate (Jaumann) | `src/material.{h,cpp}` |
| Plasticity | Johnson–Cook Sima 2010 + radial return | `src/plasticity.{h,cpp}`, `src/johnson_cook_Sima_2010.{h,cpp}` |
| Contact/friction | Tool contact penalty + friction | `src/contact.{h,cpp}`, `src/tool.{h,cpp}` |
| Thermal conduction | PSE or Brookshaw conduction; optional cooldown convection | `src/thermal.{h,cpp}` |
| Adaptivity/refinement | Particle splitting/merging logic | `src/adaptivity.{h,cpp}` |
| Output/logging | Legacy VTK writer + force/trace logs | `src/vtk_writer.{h,cpp}`, `src/logger.{h,cpp}` |
| Config system | JSON parser + typed config + config→body builder | `src/config/*`, `src/config/build_from_config.{h,cpp}` |
| Benchmarks | Built-in models used in the paper | `src/benchmarks/*` |

### 3.3 “Business logic rules” (behavioral invariants)

This is not a business application, but there are important solver invariants worth preserving:

- **Neighbor lists are rebuilt every step** before evaluating spatial operators.
- **RHS assembly order** in the stepper defines coupling (pressure/EOS → artificial stress → stress divergence → viscosity/XSPH → stress rate → continuity/momentum/advection).
- **Fixed boundary conditions** are enforced by resetting particle position/velocity for `particle.fixed` particles after correction.
- **Cooldown convection** (if enabled) currently applies to all particles using an effective area/volume proxy and a global cooling-rate cap.

## 4. Interfaces

### 4.1 CLI (“API”)

There are no HTTP API endpoints. The primary “API surface” is the CLI.

#### `mfree_iwf` CLI

Minimal benchmark run:

```powershell
.\build\Release\mfree_iwf.exe -m 1
```

Key flags (see `src/refine_cut_main.cpp`):

- `-m <1..4>`: run one of the built-in benchmark models
- `--config <path>`: run from JSON configuration
- `--dump-config <path>`: write a full default config JSON
- `--smoke`: short run intended for CI/tests
- `--cooldown`: enable cooldown stage after cutting
- `--cooldown-remove-tool`: remove tool during cooldown
- `--cooldown-hconv <W/m^2/K>`: set cooldown convection coefficient (also enables cooldown)
- `--all-steps`: write output every step

#### `view_setup` CLI

Write a single setup frame:

```powershell
.\build\Release\view_setup.exe --config configs/model1.json --out results
```

### 4.2 Library-style public headers

This repository is primarily an application, but many headers are usable as a “library interface”:

- Core types: `src/body.h`, `src/particle.h`, `src/tool.h`
- Config API: `src/config/simulation_config.h`, `src/config/build_from_config.h`, `src/config/json.h`
- Output API: `src/vtk_writer.h`, `src/logger.h`

## 5. Configuration

### 5.1 JSON configuration schema (authoritative)

See:

- [CONFIG_SCHEMA.md](CONFIG_SCHEMA.md) (field-by-field schema)
- `src/config/simulation_config.{h,cpp}` (validator and defaults)

Generate a fully populated default config:

```powershell
.\build\Release\mfree_iwf.exe --dump-config default_config.json
```

Run using a config:

```powershell
.\build\Release\mfree_iwf.exe --config configs/model1.json
```

### 5.2 Configuration categories

The JSON config is organized into:

- `io`: output controls
- `model`: scenario selection (`single_resolution`, `apriori_refinement`, `dynamic_refinement`)
- `workpiece`: geometry and discretization
- `tool`: geometry, friction, motion
- `time`: time stepping control (including CFL logic if dt not overridden)
- `numerical`: SPH/correction constants (artificial viscosity, XSPH, Monaghan stress correction)
- `material`: physical constants library selection
- `plasticity`: model toggle and parameters
- `thermal`: toggle and solver choice
- `multiresolution`: settings for models 2/3
- `adaptivity`: settings for dynamic refinement

### 5.3 Cooldown convection (runtime-only)

Cooldown convection is not currently part of the JSON config schema. It is enabled by CLI during cooldown and uses hardcoded ambient temperature defaults in `src/refine_cut_main.cpp`.

See also: `src/thermal.cpp` for the exact implemented convection term.

## 6. Data Formats

### 6.1 Legacy VTK output (ParaView)

The solver writes legacy VTK files (ASCII) for particles and the tool geometry:

- `out_000000.vtk`, `out_000001.vtk`, …
- `tool_000000.vtk`, `tool_000001.vtk`, …

These are viewable in ParaView: https://www.paraview.org/

### 6.2 Text/CSV outputs

Depending on enabled features:

- `trace.txt`: tracer particle coordinates
- `*_forces`: time and force components
- `cooldown_rate.csv`, `cooldown_summary.txt`: cooldown monitoring outputs

## 7. Dependencies

- C++17 compiler
- CMake (recommended build system)
- OpenMP (required for the main simulation target)
- GLM (math library; fetched by CMake unless disabled)

## 8. Build & Install

The recommended build system is CMake (cross-platform) with Ninja or Makefiles.

- See [BUILDING.md](BUILDING.md) for:
  - CMake installation and PATH verification (Windows/Linux/macOS)
  - Build scripts (`build.sh`, `build.bat`)
  - OpenMP notes per toolchain
  - Clean-build reproducibility steps and troubleshooting

Quick manual build:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 8
ctest --test-dir build --output-on-failure
```

## 9. Run & Deploy

### 9.1 Run modes

- Benchmark mode: `.\build\Release\mfree_iwf.exe -m <1..4>`
- Config mode: `.\build\Release\mfree_iwf.exe --config <json>`
- Smoke mode (for fast verification): `.\build\Release\mfree_iwf.exe --smoke -m 1`
- Cooldown mode: `.\build\Release\mfree_iwf.exe --cooldown --config <json>`

### 9.2 Output directories

By default, results are written to `results/` unless overridden by config (`io.output_dir`).

### 9.3 Deployment

This is a native C++ application. Typical “deployment” is distributing the built executable(s) plus any required runtime files:

- `mfree_iwf` binary
- JSON config files under `configs/` (optional)
- An output directory for generated `.vtk` / `.txt` / `.csv`

There is no server deployment, container, or database requirement.

## 10. Testing

Run all tests via CTest:

```powershell
ctest --test-dir build --output-on-failure
```

Included tests:

- `test_json_unicode`: validates JSON Unicode escape handling
- `test_property_interpolation`: validates table/linear property interpolation logic
- `smoke_model_1`: runs `mfree_iwf --smoke -m 1`
- `smoke_config_model1`: runs `mfree_iwf --smoke --config configs/model1.json`

## 11. Maintenance & Extension

### 11.1 Preserving existing functionality

Recommended workflow when making changes:

1. Keep changes isolated (one feature/fix per change-set).
2. Build and run `ctest` locally.
3. If changing physics/coupling:
   - verify at least a smoke run (`--smoke`) still produces outputs and does not diverge
   - verify the output schema remains compatible with existing post-processing (VTK fields, file naming)
4. If changing config:
   - update schema docs ([CONFIG_SCHEMA.md](CONFIG_SCHEMA.md))
   - ensure `--dump-config` continues to generate a complete default config

### 11.2 Adding new configuration parameters

Typical steps:

1. Extend `src/config/simulation_config.h` (structure + defaults).
2. Extend parsing/validation in `src/config/simulation_config.cpp`.
3. Wire into object construction in `src/config/build_from_config.cpp`.
4. Update [CONFIG_SCHEMA.md](CONFIG_SCHEMA.md).
5. Add or extend a test (preferably a CTest case or a unit test in `src/tests/`).

### 11.3 Adding new physics terms

Recommended integration pattern:

- Add term implementation in a focused module (`material`, `correctors`, `thermal`, `contact`, …).
- Add explicit coupling call in `leap_frog::step` and justify ordering.
- Prefer symmetric/consistent discretizations already used in the codebase.

### 11.4 Documentation maintenance

When making changes, update the corresponding documentation:

- Solver behavior, CLI, run modes, or dependencies → `README.md`
- JSON config schema changes → `CONFIG_SCHEMA.md` and `CONFIGURATION_GUIDE.md`
- Build system or toolchain notes → `BUILDING.md` and `README_WINDOWS.md`
- Architecture or module changes → `TECHNICAL_OVERVIEW.md`
- Thermal/mechanical coupling changes → `docs/coupling_thermal_mechanical.md`
- FE tool coupling changes → `docs/fe_tool_thermal_coupling.md`
- All significant decisions and milestone completions → `docs/work_log.md`
- Keep command examples consistent: use `powershell` code fences and `.\build\Release\*.exe` paths.

### 11.5 Commit conventions and change logging

- Use conventional commit prefixes: `docs:`, `fix:`, `feat:`, `chore:`, `refactor:`
- Keep commits scoped and atomic; separate tooling/docs changes from solver/physics changes.
- Include `Co-Authored-By: Oz <oz-agent@warp.dev>` in every commit message.
- Push to `origin master` after each completed change set.
- Before creating a feature branch, ensure `master` is clean and up to date.

### 11.6 Repository identity

This is a standalone Windows-first fork of `iwf-inspire/mfree_iwf-ul-cut-refine`. Development happens on `GS76/mfree_iwf_ul_cut_refine_windows_ver` (origin) with `upstream` retained for selective pulls. No merge-back to upstream is planned. See `docs/work_log.md` (2026-05-30 entry) for the decision record.

## 12. Troubleshooting

### Build issues

- OpenMP not found: install the toolchain OpenMP runtime and reconfigure; see [BUILDING.md](BUILDING.md).
- Mixed build directories: use separate build dirs per toolchain and configuration (e.g., `build-msvc-release`, `build-gcc-debug`).

### Runtime issues

- No output files: confirm `io.output_dir` and whether output clearing is enabled.
- Very slow runs: start with `--smoke`, reduce particle counts (`model.nbox`), and verify OpenMP is enabled.
- Cooldown not cooling: ensure `--cooldown` is set and (optionally) increase `--cooldown-hconv`.

## 13. License & Credits

This project is GPLv3 licensed.

Developed at IWF, ETH Zurich by:

- Mohamadreza Afrasiabi
- Matthias Röthlin
- Hagen Klippel

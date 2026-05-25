# AI Agent Instructions for mfree_iwf_ul_cut_refine

This C++ CMake project simulates meshfree metal cutting with SPH workpiece and FE/deformable tools. Follow these guidelines for productive contributions.

## Build & Test Commands

- **Initial setup**: `cmake -S . -B build`
- **Build Release**: `cmake --build build --config Release`
- **Run tests**: `ctest -C Release --test-dir build --output-on-failure`
- **Run validation-only executable**: `.\build\Release\mfree_iwf_validate.exe`
- **Format check**: `python scripts/check_clang_format.py`
- **EditorConfig/basic text check**: `python scripts/check_editorconfig_basic.py`

Environment variables control runtime behavior (no command-line args). See [development_workflow.md](docs/development_workflow.md) for CI and local development.

## Common Runtime / Validation Commands

- **Primary executable**: `.\build\Release\mfree_iwf.exe`
- **Preprocess-only geometry/export check**: set `MFREE_PREPROCESS_ONLY=1` before running `mfree_iwf`
- **Geometry clearance validation**: set `MFREE_GEOM_VALIDATE=1` together with `MFREE_PREPROCESS_ONLY=1`
- **Common scripted validation entrypoints**:
  - `.\\scripts\\fe_tool_bc_validate.ps1`
  - `.\\scripts\\thermal_bidir_validation.ps1`
- **Common run/visualization helpers**:
  - `.\\scripts\\run_baseline_model.ps1`
  - `.\\scripts\\run_paraview_batch_vis.ps1`

Useful runtime env vars already used across CTest and scripts:

- `MFREE_RESULTS_DIR`: redirect outputs to a dedicated results folder
- `MFREE_CLEAN_RESULTS=1`: clear the target results folder before a run
- `MFREE_NO_RIGID_TOOL=1`: enforce FE-only contact path
- `MFREE_USE_FE_TOOL_FOR_CONTACT=1`: use FE-tool boundary geometry for contact

## Architecture Overview

Three-tier system: SPH meshfree solver (workpiece particles), FE tool contact/coupling, physics modules (plasticity/thermal/adaptivity).

Sequential coupling: SPH advances first, then maps forces to FE tool. Not fully coupled timestepping.

Key interfaces: [tool_iface.h](src/tool_iface.h) for contact abstraction.

## Code Conventions

- **Naming**: snake_case classes, `m_` prefix for private members
- **Style**: Tabs (width 4), attach braces, 140 char limit, unsorted includes
- **C++**: C++17 required
- **Headers**: `FILENAME_H_` guards

Formatting enforced by [.clang-format](.clang-format) and [.editorconfig](.editorconfig).

## Common Pitfalls

- Avoid modifying [particle.h](src/particle.h) god class (50+ public members)
- Watch hardcoded limits like `MAX_NBH = 600` in [particle.h](src/particle.h)
- Use environment variables for configuration, not CLI args
- Validate geometry clearance before production runs (`MFREE_PREPROCESS_ONLY=1`)

See [refractor_suggestions.md](docs/refractor_suggestions.md) for known issues.

## Key Patterns

- Environment-driven config: `env_flag()`, `env_int()`, `env_double()` in [refine_cut_main.cpp](src/refine_cut_main.cpp)
- Template solvers: [solver.h](src/solver.h)
- VTK output: [vtk_writer.cpp](src/vtk_writer.cpp)
- Spatial acceleration: [grid.cpp](src/grid.cpp)
- CTest coverage includes preprocess and geometry-validation runs for the active Model 5 FE-tool workflow in `CMakeLists.txt`

For physics details, see [coupling_thermal_mechanical.md](docs/coupling_thermal_mechanical.md) and [fe_tool_thermal_coupling.md](docs/fe_tool_thermal_coupling.md).

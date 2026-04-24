# AI Agent Instructions for mfree_iwf_ul_cut_refine

This C++ CMake project simulates meshfree metal cutting with SPH workpiece and FE/deformable tools. Follow these guidelines for productive contributions.

## Build & Test Commands

- **Initial setup**: `cmake -S . -B build`
- **Build Release**: `cmake --build build --config Release`
- **Run tests**: `ctest -C Release --test-dir build --output-on-failure`
- **Format check**: `python scripts/check_clang_format.py`

Environment variables control runtime behavior (no command-line args). See [development_workflow.md](docs/development_workflow.md) for CI and local development.

## Architecture Overview

Three-tier system: SPH meshfree solver (workpiece particles), rigid/deformable tools (contact), physics modules (plasticity/thermal/adaptivity).

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

For physics details, see [coupling_thermal_mechanical.md](docs/coupling_thermal_mechanical.md) and [fe_tool_thermal_coupling.md](docs/fe_tool_thermal_coupling.md).</content>
<parameter name="filePath">d:\mfree_iwf_ul_cut_refine_windows_ver\AGENTS.md
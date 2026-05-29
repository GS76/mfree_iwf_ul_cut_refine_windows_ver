# FE Tool vs Workpiece Material Initialization

This note documents how material properties are initialized for:

- FE tool (finite-element tool body), and
- workpiece (SPH particles).

It focuses on Model 5 FE-tool-only coupled workflow.

## Related Documents

- [FE Tool Temperature-Dependent Material (Linear Thermoelastic)](fe_tool_temperature_dependent_material.md)
- [Thermal + Mechanical Coupling (SPH Workpiece ↔ FE Tool)](coupling_thermal_mechanical.md)
- [FE Tool Thermal Coupling (SPH Workpiece ↔ Meshed Tool)](fe_tool_thermal_coupling.md)

## Summary

- FE tool material is initialized primarily from runtime environment variables (with hardcoded defaults), then optional temperature tables.
- Workpiece material is initialized from a compiled material-library preset (`physical_constants`), then optionally modified by SPH temperature tables.
- FE tool supports direct scalar runtime overrides for `E` and `nu`; workpiece does not use the same direct scalar override path in this workflow.

## FE Tool Initialization Path

Entry point: `src/benchmarks/test_cuttings.cpp`, function `attach_fe_tool_from_env(...)`.

### 1) Mesh and FE object creation

- Reads `MFREE_FE_TOOL_MSH` (fallback to `./snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh`).
- Creates and loads `fe_tool`.

### 2) Thermal material defaults and scalar overrides

Initial defaults:

- `rho = 14500.0`
- `cp = 200.0`
- `k = 80.0`

Scalar env overrides (if present):

- `MFREE_FE_TOOL_RHO`
- `MFREE_FE_TOOL_CP`
- `MFREE_FE_TOOL_K`

Applied via `ft->set_material(...)`.

### 3) Thermal temperature tables (optional)

If provided, these tables are parsed and assigned:

- `MFREE_FE_TOOL_RHO_TABLE` (alias: `MFREE_FE_RHO_TABLE`)
- `MFREE_FE_TOOL_CP_TABLE` (alias: `MFREE_FE_CP_TABLE`)
- `MFREE_FE_TOOL_K_TABLE` (alias: `MFREE_FE_K_TABLE`)

### 4) Mechanical material defaults and scalar overrides

Initial defaults:

- `E = 600e9`
- `nu = 0.22`
- `alpha = 4.5e-6`

Scalar env overrides (if present):

- `MFREE_FE_TOOL_E`
- `MFREE_FE_TOOL_NU`
- `MFREE_FE_TOOL_ALPHA`

Applied via `ft->set_mechanical_material(...)`.

Validation guard in `src/fe_tool.cpp` rejects invalid input:

- `E > 0`
- `-1 < nu < 0.5`
- `alpha >= 0`

### 5) Mechanical temperature tables (optional)

If provided, these tables are parsed and assigned:

- `MFREE_FE_TOOL_E_TABLE` (alias: `MFREE_FE_E_TABLE`)
- `MFREE_FE_TOOL_NU_TABLE` (alias: `MFREE_FE_NU_TABLE`)
- `MFREE_FE_TOOL_ALPHA_TABLE` (alias: `MFREE_FE_ALPHA_TABLE`)

When mechanical tables are present, FE mechanics can be rebuilt using `E(T)` / `nu(T)` during runtime.

## Workpiece SPH Initialization Path

Entry path: cutting setup in `src/benchmarks/test_cuttings.cpp` selecting a material preset such as:

- `matlib_tial6v4_Sima_tanh2010_SI()`

Defined in `src/benchmarks/material_library.cpp`, returned as `physical_constants`.

### 1) Material-library preset

The preset defines baseline material constants (including `E`, `nu`, `rho0`, Johnson-Cook constants, thermal constants), then the simulation stores them in `simulation_data`.

Derived constants (e.g. `G`, `K`, Lamé parameters) are computed in `src/simulation_data.cpp` from baseline `E` and `nu`.

### 2) Optional SPH temperature-dependent stiffness tables

In `src/material.cpp`, SPH constitutive update reads optional workpiece tables:

- `MFREE_WP_E_TABLE` (alias: `MFREE_WORKPIECE_E_TABLE`)
- `MFREE_WP_G_TABLE` (alias: `MFREE_WORKPIECE_G_TABLE`)

Runtime stress-rate update uses temperature-dependent shear modulus `G(T)` via these tables (falling back to baseline when absent/invalid).

## Key Differences

1. Source of truth
   - FE tool: runtime env-first (defaults + env overrides + env tables).
   - Workpiece: material-library preset first, optional table modulation.

2. Scalar runtime overrides for elasticity
   - FE tool: direct scalar env knobs for `E` and `nu`.
   - Workpiece: no equivalent direct scalar `E/nu` env initialization path in the main Model 5 setup.

3. Numerical role
   - FE tool: FE operators assembled from FE thermal/mechanical material structures.
   - Workpiece: SPH constitutive response driven by `physical_constants` and SPH stress update kernels.

4. Table usage
   - FE tool: dedicated FE-tool thermal + mechanical table families (with FE aliases).
   - Workpiece: SPH workpiece stiffness tables focused on `E/G` behavior.

## Practical Implication

For FE-tool studies, changing material behavior is typically fastest through FE env variables/tables.

For workpiece studies, baseline behavior is best changed by selecting/updating the material-library preset, with `MFREE_WP_*_TABLE` used for temperature-dependent stiffness adjustments.

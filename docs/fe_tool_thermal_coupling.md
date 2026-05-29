# FE Tool Thermal Coupling (SPH Workpiece ↔ Meshed Tool)

## Overview

This codebase uses a thermally active cutting tool represented by a finite element (FE) surface/volume mesh with an explicit thermal solver. Contact geometry and coupling are FE-tool-based for the forward workflow, and thermal exchange plus frictional heat are transferred into the FE tool mesh conservatively (equal and opposite power on SPH vs FE).

## Related Documents

- [FE Tool vs Workpiece Material Initialization](material_initialization_fe_tool_vs_workpiece.md)
- [FE Tool Temperature-Dependent Material (Linear Thermoelastic)](fe_tool_temperature_dependent_material.md)
- [Thermal + Mechanical Coupling (SPH Workpiece ↔ FE Tool)](coupling_thermal_mechanical.md)

Core implementation:

- FE tool thermal state + explicit conduction + convection: [fe_tool.h](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.h), [fe_tool.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp)
- Thermal contact conductance + frictional heat partition applied during contact: [contact.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp)
- Time-loop integration point (preserves neighbor rebuild ordering): [leap_frog.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/leap_frog.cpp)

## Enabling The FE Tool

The cutting benchmarks can optionally attach an FE tool by setting an environment variable:

- `MFREE_FE_TOOL_MSH`: path to a Gmsh v2 ASCII `.msh` file containing the tool mesh
- `MFREE_USE_FE_TOOL_FOR_CONTACT`: if set to nonzero, contact geometry is constructed from FE tool boundary edges (piecewise-linear polygon)
- `MFREE_COOLANT_Y_THRESHOLD` (optional): world-space y threshold; boundary edges with midpoint `y >= threshold` use the flooded-water convection model, below use still air

Optional deformable plane-strain response (quasi-static):

- `MFREE_DEFORMABLE_FE_TOOL`: if set to nonzero together with `MFREE_USE_FE_TOOL_FOR_CONTACT`, iterates contact against the deformed FE-tool boundary and solves a plane-strain thermoelastic equilibrium for the tool under mapped nodal forces (thermal expansion uses `MFREE_FE_TOOL_ALPHA` and the current FE temperature field)
- `MFREE_DEFORMABLE_TOOL_TOL`: relative tolerance for mapped nodal force and mapped nodal power residuals
- `MFREE_DEFORMABLE_TOOL_MAX_ITERS`: maximum contact iterations
- `MFREE_DEFORMABLE_TOOL_RELAX`: displacement under-relaxation (0..1)

Run:

- `mfree_iwf.exe -m 5`

Attachment wiring is in [test_cuttings.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/test_cuttings.cpp).

## Mesh Input Requirements

Supported mesh format:

- Gmsh v2 ASCII `.msh`
- 2D triangles (element type 2) for the thermal conduction domain
- Boundary line elements (element type 1) are used as boundary edges; the first tag is treated as the “physical tag”

Material overrides via environment variables:

- `MFREE_FE_TOOL_RHO`, `MFREE_FE_TOOL_CP`, `MFREE_FE_TOOL_K`
- `MFREE_FE_TOOL_E`, `MFREE_FE_TOOL_NU`, `MFREE_FE_TOOL_ALPHA`

## Thermal Models

### FE Tool Conduction

The FE tool solves:

- `rho * cp * dT/dt = div(k grad T) + q`

with:

- P1 triangles, assembled stiffness operator
- lumped heat capacity per node
- explicit time advance

### Conservative SPH↔FE Flux Coupling

For each contacting particle, thermal exchange is computed at the FE-tool contact point and applied as:

- particle temperature rate: `T_t += (-P_cond + f_wp * P_fric) / (m * cp)`
- tool nodal power source: `P_cond + f_tool * P_fric` distributed to the nearest FE boundary edge by linear shape weights

Default: `f_wp = 0.8`, `f_tool = 0.2`.

### Pressure-Dependent Thermal Contact Conductance

Contact conductance is mapped as:

- `h_c(p) = h_sep + (h_full - h_sep) * clamp(p / p_ref, 0, 1)`
- defaults:
  - `h_sep = 1000 W/m²K` (`MFREE_THERMAL_H_SEP`)
  - `h_full = 100000 W/m²K` (`MFREE_THERMAL_H_FULL`)
  - `p_ref = 1e9 Pa` (`MFREE_THERMAL_P_REF`)

Pressure estimate uses the penalty normal force magnitude and an SPH “effective area” `A_eff ≈ m/rho` (2D unit thickness assumption).

Interpretation:

- `h_sep` is the effective interface conductance when contact pressure is near zero (near-separated asperity contact).
- `h_full` is the upper bound conductance at high pressure (intimate contact limit).
- `p_ref` is the pressure scale where the transition reaches the upper bound (`p >= p_ref` ⇒ `h_c = h_full`).

### Frictional Heating

Frictional power is computed as:

- `P_fric = |F_t| * |v_rel,t|`

and partitioned:

- 0.8 to workpiece
- 0.2 to tool

The partition fractions can be overridden by environment variables:

- `MFREE_THERMAL_FRAC_WP`: workpiece fraction
- `MFREE_THERMAL_FRAC_TOOL`: tool fraction

If only one is specified, the other is taken as `1 - specified`. If both are specified, they are normalized to sum to 1.

### 1°C/Step Interface Limiter

The coupling applies a global limiter so the largest predicted SPH temperature increment from interface exchange does not exceed 1°C per step.

Override:

- `MFREE_THERMAL_MAX_DT_PER_STEP` (Kelvin)

## Convection Boundary Conditions

Tool convection is supported via:

- Still air: `h = 20 W/m²K`, `T_inf = 298.15 K`
- Flooded water: `h = 5000 W/m²K`, `T_inf = 293.15 K`

The current “automatic” detection is a geometric rule by boundary-edge midpoint y-coordinate (see `MFREE_COOLANT_Y_THRESHOLD`).

## Validation

Build + run the validation suite:

- `cmake -S . -B build`
- `cmake --build build --config Release`
- `ctest --test-dir build -C Release --output-on-failure`

Validation executable:

- [validate_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/benchmarks/validate_main.cpp)

Checks include:

- transient conduction (tool)
- frictional heating partition energy balance (tool/workpiece)
- convection cooling against a lumped reference

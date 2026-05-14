# Coupled thermal-structural timestep estimation

The cutting setups now use a central timestep estimator in `src/timestep_estimator.h` / `src/timestep_estimator.cpp` for the SI-unit FE-tool cases. The estimator returns the minimum stable timestep over the active workpiece, FE-tool, and contact-interface limits.

## Limits considered

For the SPH workpiece, the estimator computes:

- mechanical/acoustic limit using the workpiece elastic wave speed `physical_constants::c0()` and the largest expected relative cutting speed;
- thermal diffusion limit using the workpiece thermal diffusivity `k / (rho * cp)` from `physical_constants::tc()`.

For the FE tool, when a tool is available, it computes:

- explicit structural limit from `fe_tool::mechanics_dt_crit()`, which accounts for the current FE mesh size and temperature-dependent `rho`, `E`, and `nu` material tables;
- explicit thermal limit from `fe_tool::thermal_dt_crit()`, which accounts for the current FE conduction operator, lumped capacities, convection, and Dirichlet boundaries;
- interface thermal exchange limit for a two-capacity contact pair using the SPH particle heat capacity and the minimum FE tool nodal heat capacity.

The interface estimate uses the full-contact conductance as the conservative contact conductance and the effective contact area, defaulting to one SPH particle area.

## Runtime controls

The cutting setup helper `estimate_dt_for_cutting()` honors these environment variables:

- `MFREE_TIMESTEP_WP_MECH_SAFETY`
- `MFREE_TIMESTEP_WP_THERM_SAFETY`
- `MFREE_TIMESTEP_TOOL_MECH_SAFETY`
- `MFREE_TIMESTEP_TOOL_THERM_SAFETY`
- `MFREE_TIMESTEP_INTERFACE_SAFETY`
- `MFREE_TIMESTEP_INTERFACE_AREA_FACTOR`
- `MFREE_THERMAL_H_FULL`
- `MFREE_TIMESTEP_PRINT=0` to suppress printed timestep reports

The estimator is called once before the FE tool is attached, then again after the FE tool is loaded and positioned so the final timestep includes the actual FE material tables, FE mesh, and interface capacity scale.

## Output

When printing is enabled, the run emits a line like:

`timestep estimate: dt=... limiter=... wp_mech=... wp_therm=... tool_mech=... tool_therm=... interface_therm=... empirical=...`

The `limiter` field identifies which physical process selected the final maximum timestep.

# Output Units Consistency (Model 5 FE-tool Coupled Workflow)
This note documents the units of exported output fields and verifies consistency against governing equations and data structures.
## Scope
- Workpiece VTK output fields (`src/vtk_writer.cpp`)
- FE-tool VTK output fields (`src/vtk_writer.cpp`)
- Time-series CSV outputs:
  - `*_thermal.csv`
  - `*_metrics.csv`
  - `*_energy.csv`
## Base Unit System
The active forward workflow is SI-consistent:
- Length: m
- Mass: kg
- Time: s
- Temperature: K
Mechanical stress/modulus fields are in Pa.
## Workpiece VTK Fields
Source: `src/vtk_writer.cpp`
- `density`: kg/m^3
- `temperature`: K
- `Svm`: Pa
- `equiv_plastic_strain`: dimensionless
- `velocity`: m/s
- `contact_force_n`, `contact_force_t`: N
- `contact_pressure`: Pa
- `displacement`: m
- `glob_density_err`: (kg/m^3)^2 (squared density error)
- `mass`: kg
- `fixed`: dimensionless flag (0/1)
- `num_neighbors`: count
- `refine_step`: refinement level index (dimensionless integer)
- `initial_x`, `initial_y`: m
- `initial_temperature`: K
- `T_t`: K/s
## FE-tool VTK Fields
Source: `src/vtk_writer.cpp`
- `temperature`: K
- `power`: W
- `nodal_force`: N
- `pose_velocity`: m/s
- `fixed_ux`, `fixed_uy`: dimensionless flags (0/1)
## Thermal CSV (`*_thermal.csv`)
Header source: `src/logger.cpp`
- `time`: s
- `step`: count
- `P_cond_W`, `P_fric_W`: W
- `scale`, `frac_wp`, `frac_tool`: dimensionless
- `tool_pos_x`, `tool_pos_y`: m
- `tool_vel_x`, `tool_vel_y`: m/s
- `tool_Tmin`, `tool_Tmax`, `wp_Tmin`, `wp_Tmax`, `wp_Tavg`: K
- `contact_iters`: count
- `rel_force`, `rel_power`: dimensionless convergence metrics
## Metrics CSV (`*_metrics.csv`)
Header source: `src/logger.cpp`
- `time`: s
- `step`: count
- `wp_Tmin`, `wp_Tmax`, `wp_Tavg`: K
- `wp_umax`: m
- `wp_svm_max`: Pa
- `wp_epspl_max`: dimensionless
- `wp_contact_pmax`: Pa
- `wp_contact_count`: count
## Energy CSV (`*_energy.csv`)
Header and computation sources: `src/logger.cpp`, `src/contact.cpp`, `src/fe_tool.cpp`
- `time`: s
- `step`: count
- `step_dt`: s
- `*_P_*` fields: W
- `*_E_*`, `delta_*`, `closure_residual`: J
- `closure_residual_pct`: percent (%)
- `step_contact_area_eff`: m^2
- `step_contact_hA`: W/K
- `step_contact_h_c_*`: W/(m^2*K)
- `step_contact_deltaT_*`, `step_contact_max_pred_dT`, `T_ref`: K
- ratios/fractions (`*_ratio`, `*_frac`, `scale`): dimensionless
## Consistency Verdict
All exported fields above are internally consistent with SI units and with the implemented thermomechanical/contact formulations.
## Important Caveat (Plane-Strain Thickness Scaling)
When `MFREE_PLANE_STRAIN_THICKNESS` is changed from its implicit unit-depth convention:
- Interface and radiation power terms explicitly include thickness (e.g., contact and radiation source terms).
- FE thermal capacity assembly uses in-plane area with implicit unit depth.
Units remain SI-consistent, but absolute thermal source/storage scaling can become physically inconsistent unless thickness treatment is applied consistently across all related terms.
## References
- `src/vtk_writer.cpp`
- `src/logger.cpp`
- `src/contact.cpp`
- `src/fe_tool.cpp`
- `src/body.cpp`
- `src/simulation_data.h`

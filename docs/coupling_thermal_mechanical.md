# Thermal + Mechanical Coupling (SPH Workpiece ↔ FE Tool)

This document describes the thermal and mechanical coupling implemented between the SPH workpiece (“body/particles”) and the FE tool (“fe_tool”) in the current solver, including coupling modes, execution order, exchanged quantities, and main trade-offs.

## High-Level Execution Order (Per Global SPH Time Step)

The SPH time integration uses a leapfrog predictor/corrector. Coupling is executed as part of the SPH step, not as a separate process.

Per step (simplified), the order is:

1. SPH neighbor rebuild (`construct_verlet_lists`)
2. SPH predictor (`init`, `predict`)
3. Reset SPH derivatives (per-particle `reset`)
4. Contact + coupling (`body.apply_contact`)
5. (Optional) FE tool explicit mechanics update (`body.advance_fe_tool_mechanics_explicit`)
   - This is a no-op when explicit coupled mode is active.
6. Tool kinematic update (`body.move_tool`)
7. SPH mechanical derivatives (EOS, stress, momentum, etc.)
8. SPH thermal conduction (`body.apply_thermal_conduction`)
9. (Optional) FE tool thermal update (`body.advance_fe_tool_thermal`)
   - This is a no-op when explicit coupled mode is active.
10. SPH corrector (`correct`)
11. Plasticity, BCs, adaptivity

Code references:
- Step sequence: [leap_frog::step](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/leap_frog.cpp#L108-L160)
- Coupling entry point: [body::apply_contact](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L144-L399)

## Coupling Modes (What Runs When)

There are three main tool/contact setups, plus two deformable-tool coupling modes.

| Mode | Enabled By | Tool Geometry Used For Contact | FE Tool Mechanics | FE Tool Thermal | Notes |
|---|---|---|---|---|---|
| Rigid analytic tool | no FE tool attached | `tool` polygon | N/A | N/A | SPH contact only. |
| FE tool attached, rigid in mechanics | `MFREE_USE_FE_TOOL_FOR_CONTACT=0` OR `MFREE_USE_FE_TOOL_FOR_CONTACT=1` + `MFREE_DEFORMABLE_FE_TOOL=0` | Either analytic `tool` or FE boundary polygon | No mechanics solve | Yes (can receive heat) | FE tool is used mainly as a thermal body (and for contact geometry if enabled). |
| Deformable FE tool (quasi-static coupling) | `MFREE_DEFORMABLE_FE_TOOL=1` and `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=0` | FE boundary polygon | Quasi-static solve per contact iteration | Thermal load mapping exists, but thermal advance is not part of the inner quasi-static loop | Iterative contact loop enforces force/power convergence with optional relaxation. |
| Deformable FE tool (explicit coupled) | `MFREE_DEFORMABLE_FE_TOOL=1` and `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1` | FE boundary polygon | Explicit mechanics substepping inside `apply_contact()` | Explicit thermal substepping inside `apply_contact()` | Strong serial coupling inside a single SPH step; later per-step FE calls are skipped. |

## Mechanical Coupling Details

### Workpiece (SPH) Side

For each SPH particle inside the tool, the solver computes:

- Normal contact force (`fcx/fcy`) from a penalty-type formulation (optionally with a Lagrange-multiplier-like accumulator for the normal component).
- Tangential/friction force (`ftx/fty`) from a friction law using the tool velocity and the particle velocity.

These forces enter the SPH momentum equation later in the same time step.

Code reference: [contact_apply_tool_to_body_2d](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp#L331-L435)

### FE Tool Side (Reaction Forces)

When an FE tool is present for coupling, the solver applies equal-and-opposite contact reactions to the FE tool boundary:

- `F_tool = -(F_normal + F_friction)`
- The reaction is distributed to the nearest FE boundary edge nodes via barycentric weighting.

Code references:
- Reaction force mapping: [contact.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp#L462-L467)
- FE boundary force distribution: [fe_tool::add_boundary_point_force](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L344-L357)

### FE Tool Mechanics Solver Options

Two mechanics solvers exist:

- **Explicit mechanics** (`advance_mechanics_explicit(dt)`): lumped-mass explicit update with optional Rayleigh damping.
  Code reference: [fe_tool::advance_mechanics_explicit](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L1237-L1353)
- **Quasi-static mechanics** (`solve_mechanics_quasistatic(...)`): iterative solve to equilibrium under applied nodal forces.
  Code reference: [fe_tool::solve_mechanics_quasistatic](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L1355)

In deformable-tool mode, `body.apply_contact()` selects:

- **Explicit coupled mode**: advances FE mechanics in substeps inside the contact/coupling loop.
  Code reference: [body.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L234-L307)
- **Quasi-static coupled mode**: advances FE mechanics each iteration via `solve_mechanics_quasistatic`, optionally relaxes displacements, and checks convergence based on nodal force/power changes.
  Code reference: [body.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L309-L399)

## Thermal Coupling Details

Thermal coupling is implemented as an interfacial power exchange between workpiece particles and the FE tool boundary:

1. **Conductive exchange** driven by workpiece vs tool temperature difference, with a pressure-dependent contact conductance `h_c`.
2. **Frictional heating** computed from tangential friction force magnitude and slip speed; a user-controlled fraction goes to each side.
3. **Safety scaling** to limit the maximum predicted workpiece temperature increment per global step.

### What Is Exchanged

| Quantity | From | To | How It Is Used |
|---|---|---|---|
| `F_normal`, `F_friction` | Contact model | SPH particles | Added to SPH momentum equation in the same step. |
| `- (F_normal + F_friction)` | Contact model | FE tool boundary | Added as nodal forces; affects FE displacement update. |
| `P_cond` (conduction power) | `T_wp - T_tool` | Both sides | Removes energy from hotter side, adds to colder side (sign handled in formulas). |
| `P_fric` (friction power) | Slip * friction force | Both sides | Split by fractions `frac_wp`, `frac_tool`. |
| `dT_t` (increment to particle temperature derivative) | Coupling model | SPH particles | Added to `T_t`, later integrated by leapfrog. |
| `P_tool` (nodal power source) | Coupling model | FE tool boundary | Added to FE thermal RHS as `m_power_sources`. |

### Thermal Coupling Implementation

For each contact event / particle:

- Effective contact area: `A_eff = m / rho`
- Contact pressure estimate: `pressure = |F_normal| / A_eff`
- Contact conductance interpolation:
  - `s = clamp(pressure / p_ref, 0..1)`
  - `h_c = h_sep + (h_full - h_sep) * s`
- Conduction power (workpiece → tool): `P_cond = h_c * A_eff * (T_wp - T_tool)`
- Friction power: `P_fric = |F_fric| * slip_speed`
- Scale factor `scale` reduces `P_cond` and `P_fric` if the maximum predicted per-step ΔT would exceed `max_dT_per_step_K`.

Then:

- Workpiece temperature derivative increment:
  - `dT_t = (-P_cond + frac_wp * P_fric) / (m * cp_wp)`
  - added to particle `T_t`
- Tool thermal power source:
  - `P_tool = P_cond + frac_tool * P_fric`
  - mapped to FE boundary nodes with `add_boundary_point_power`

Code references:
- Thermal coupling loop and scaling: [contact.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp#L469-L564)
- FE tool boundary power distribution: [fe_tool::add_boundary_point_power](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L315-L328)
- FE tool explicit thermal update uses `m_power_sources`: [fe_tool::advance_explicit](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L526-L594)

### Thermal Coupling Controls (Env Vars)

These env vars are read once and cached for the run:

| Env Var | Meaning | Default |
|---|---|---|
| `MFREE_THERMAL_H_SEP` | `h_sep` (W/m²K) in separated contact | 1000 |
| `MFREE_THERMAL_H_FULL` | `h_full` (W/m²K) in full contact | 100000 |
| `MFREE_THERMAL_P_REF` | Reference pressure `p_ref` (Pa) for `h_c` interpolation | 1e9 |
| `MFREE_THERMAL_FRAC_WP` | Friction heat fraction to workpiece (`frac_wp`) | 0.8 |
| `MFREE_THERMAL_FRAC_TOOL` | Friction heat fraction to tool (`frac_tool`) | 0.2 |
| `MFREE_THERMAL_MAX_DT_PER_STEP` | Max predicted per-step workpiece ΔT cap (K) | 1.0 |

Code reference: [load_thermal_contact_coupling_params](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/contact.cpp#L110-L209)

## Explicit Coupled Mode: Substepping and Synchronization

When `MFREE_DEFORMABLE_FE_TOOL=1` and `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1`, coupling is performed as a serial strong-coupling loop inside `body.apply_contact()`:

- FE tool thermal is advanced with `dt_th = dt / thermal_substeps`
- FE tool mechanics is advanced with `dt_mech = dt / mech_substeps`
- Contact is re-evaluated each substep against the (potentially updated) FE boundary shape.
- The SPH particle contact force and contact-induced `T_t` are accumulated across substeps and averaged back to the SPH state for the rest of the SPH step.

Substeps are chosen by:

- `mech_substeps`:
  - forced by `MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS`, or
  - computed from FE explicit stability estimate `mechanics_dt_crit()` with a `0.9` safety factor, capped by `MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS`
- `thermal_substeps`:
  - forced by `MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS`, otherwise equals `mech_substeps`

Code reference: [body.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L214-L307)

## Advantages and Disadvantages

### Advantages

- **Single-executable coupled run**: SPH and FE domains exchange forces and heat every step with a clear and deterministic execution order.
- **Strong coupling option (explicit coupled mode)**: repeating contact + FE updates inside a substep loop improves consistency between interface loads and tool state within a single SPH timestep.
- **Robust thermal stabilization**: contact heat exchange is capped by `MFREE_THERMAL_MAX_DT_PER_STEP`, which can prevent runaway temperatures from a single step.
- **Flexible FE tool behavior**: tool can be rigid, thermal-only, quasi-static deformable, or explicit deformable depending on env vars.
- **Natural “reaction” coupling**: action/reaction forces are mapped to the FE boundary automatically from the same contact events used on the SPH side.

### Disadvantages / Limitations

- **Not concurrent**: SPH and FE do not solve in parallel or simultaneously in wall-clock time; coupling is serial within one process.
- **Operator splitting**: in the global step, contact/FE updates happen before SPH derivative evaluation; this is not a monolithic solve and can introduce splitting error.
- **Thermal coupling is one-way per substep**: `T_tool` is sampled from the FE boundary for contact heat transfer, then tool power is applied; the FE thermal state only updates when `advance_explicit()` runs.
- **Effective contact area approximation**: `A_eff = m/rho` is a heuristic and may not represent true contact patch geometry, affecting `pressure` and thus `h_c`.
- **Penalty contact sensitivity**: contact stiffness and stability depend on penalty settings (`MFREE_CONTACT_ALPHA*`) and timestep; aggressive dt increases can destabilize contact.
- **Substepping cost**: explicit coupled mode improves stability/consistency but increases compute cost per global step.

## Key Coupling/Mode Env Vars (Quick Reference)

| Env Var | Role |
|---|---|
| `MFREE_USE_FE_TOOL_FOR_CONTACT` | If 1, use FE tool boundary polygon for contact geometry and write `tool_*.vtk` from FE tool boundary as well. |
| `MFREE_DEFORMABLE_FE_TOOL` | Enables deformable FE tool coupling logic in `body.apply_contact()`. |
| `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT` | Selects explicit coupled mode (substepped FE thermal + mechanics inside contact). If 0, uses quasi-static iterations. |
| `MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS` | Overrides FE mechanics substep count in explicit coupled mode. |
| `MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS` | Overrides FE thermal substep count in explicit coupled mode. |
| `MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS` | Caps substeps when auto-computed from `mechanics_dt_crit()`. |
| `MFREE_FE_TOOL_RAYLEIGH_A0`, `MFREE_FE_TOOL_RAYLEIGH_A1` | Rayleigh damping in FE explicit mechanics. |
| `MFREE_THERMAL_*` | Interfacial thermal coupling coefficients and friction heat split (see table above). |

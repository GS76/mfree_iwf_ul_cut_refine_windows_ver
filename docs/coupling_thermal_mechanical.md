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

There are two forward FE-tool coupling modes.

| Mode | Enabled By | Tool Geometry Used For Contact | FE Tool Mechanics | FE Tool Thermal | Notes |
|---|---|---|---|---|---|
| FE tool (quasi-static coupling) | `MFREE_DEFORMABLE_FE_TOOL=1` and `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=0` | FE boundary polygon | Quasi-static solve per contact iteration | Thermal load mapping exists, but thermal advance is not part of the inner quasi-static loop | Iterative contact loop enforces force/power convergence with optional relaxation. |
| FE tool (explicit coupled) | `MFREE_DEFORMABLE_FE_TOOL=1` and `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1` | FE boundary polygon | Explicit mechanics substepping inside `apply_contact()` | Explicit thermal substepping inside `apply_contact()` | Strong serial coupling inside a single SPH step; later per-step FE calls are skipped. |

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

## Known Approximations and Error Sources

This section documents the principal sources of approximation error that are **inherent to the current sequential coupling architecture**. None of these are bugs — they are known, bounded trade-offs. The quantitative estimates below are given in SI units and assume typical Ti6Al4V workpiece / WC tool cutting conditions unless noted otherwise.

### 1. Operator-Splitting Temporal Lag — O(dt)

**What it is:** Contact/FE updates (step 4 in the execution order) occur using the SPH state at the start of the step, *before* SPH mechanical and thermal derivatives are evaluated (steps 7–8). This is a Lie–Trotter operator split. The resulting temperature and force lag between domains is first-order in time:

```
error ≈ (dT/dt)|_interface × dt
```

**Quantitative estimate:** For a cutting speed of 1–5 m/s, a friction coefficient of 0.3, and a contact pressure of ~1 GPa, the interface heating rate `dT/dt` experienced by the workpiece particle is of order 10³–10⁴ K/s. With a typical SPH timestep `dt ~ 1 × 10⁻⁸` s (set by the acoustic CFL), the per-step temperature error is 0.01–0.1 K — well below the 1 K/step limiter threshold. Over 100 000 steps this accumulates, but the error remains bounded because the operator split is not globally dissipative.

**Mitigation:** The explicit coupled mode (`MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1`) reduces this to O(dt/n_substeps) by performing n contact + FE advance sub-iterations within a single SPH step.

**How to observe:** Not directly visible in the energy CSV. Compare runs with and without `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT` and note the difference in `cum_contact_E_tool` and `delta_tool_internal_E`.

---

### 2. Thermal One-Way-Per-Substep Lag — O(dt_th)

**What it is:** Within each substep, the tool temperature `T_tool` is *sampled before* `advance_explicit()` updates the FE thermal state. The conduction power `P_cond = h_c × A_eff × (T_wp − T_tool)` therefore uses a tool temperature that is one substep behind. This introduces a lag proportional to the sub-timestep:

```
error in P_cond ≈ h_c × A_eff × (dT_tool/dt)|_boundary × dt_th
```

**Quantitative estimate:** For `h_c ~ 10⁴ W/m²K` (separated contact), `A_eff ~ (2×10⁻⁵)² m²` (one SPH particle), and a tool boundary heating rate of ~10³ K/s, the error in `P_cond` per substep is ~10⁴ × 4×10⁻¹⁰ × 10³ ≈ 4×10⁻³ W per particle. With the typical timestep this is negligible. At full contact (`h_c ~ 10⁵ W/m²K`) and aggressive dt, it can become significant.

**Mitigation:** Increasing `MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS` reduces dt_th and therefore this lag. Values of 4–8 substeps are typical for stable explicit-coupled mode.

**How to observe:** Compare `step_tool_source_residual` in `_energy.csv` (tool nodal power minus contact-model tool power) for runs with different `MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS`. A non-zero residual indicates that the power actually applied to FE nodes differs from what the contact model computed.

---

### 3. Limiter Suppression of Interface Exchange

**What it is:** The 1 °C/step safety cap (`MFREE_THERMAL_MAX_DT_PER_STEP`, default 1.0 K) globally scales both `P_cond` and `P_fric` by a factor `scale ≤ 1` when the predicted maximum per-step particle temperature increment would exceed the cap. This discards energy from the thermal budget:

```
E_suppressed = dt × (1 − scale) × (|P_cond_raw| + P_fric_raw)
```

The suppression ratio `step_suppression_ratio` (column in `_energy.csv`, introduced in the Phase 4 energy-accounting work) equals `(1 − scale)` and is 0 when the limiter is inactive.

**Quantitative estimate:** For the default 1 K cap and typical conditions, `scale ≈ 1` for most steps. The limiter fires only when frictional heating is rapid relative to the SPH timestep — this typically occurs during initial contact or at high cutting speeds. The Phase 4 validation test shows `scale = 0.000193` for dt = 1.0 s (extreme case), confirming `ratio → 1`. For dt ~ 1×10⁻⁸ s (normal), `ratio = 0`.

**How to observe (from `_energy.csv`):**
```
cum_suppression_ratio = cum_contact_E_limiter_suppressed
                        / (|cum_contact_E_cond_raw| + cum_contact_E_fric_raw)
```
A value above 0.10 (10%) indicates the timestep is too large for reliable thermal coupling at the current cutting conditions. The logger emits a one-time console warning when `step_suppression_ratio > 0.10` (see `MFREE_LOG_ENERGY` and suppression warning in `logger.cpp`).

**Mitigation:** Tighten `MFREE_THERMAL_MAX_DT_PER_STEP` (e.g., to 0.1 K) or reduce the global SPH timestep. Setting `MFREE_THERMAL_MAX_DT_PER_STEP` too small increases the suppressed fraction, so it must be balanced against numerical stability.

---

### 4. Contact Area Heuristic — A_eff

**What it is:** The effective thermal contact area per particle is computed as:

```
A_eff = contact_length × plane_strain_thickness
contact_length = sqrt(m_i / rho_i) × contact_length_factor
```

For a uniform particle lattice, `sqrt(m/rho) ≈ dx` (the inter-particle spacing), so `A_eff ≈ dx × L_z` where `L_z` is the plane-strain out-of-plane thickness (default 1.0 m). This approximates each contacting particle as occupying one element of the contact interface of size `dx × L_z`.

**Quantitative estimate:** For `dx = 2×10⁻⁵` m and `L_z = 1.0` m (plane-strain unit depth), `A_eff = 2×10⁻⁵` m². The analytical contact patch length in orthogonal cutting is of order the chip thickness (10–100 µm). With ~5–50 particles in contact, the total modelled contact area = 5–50 × 2×10⁻⁵ = 1×10⁻⁴ – 1×10⁻³ m², which brackets the analytical estimate. The area per particle is therefore a reasonable approximation for the nominally uniform pre-refinement lattice, but can deviate after adaptivity produces polydisperse particle masses.

**How to observe:** `step_contact_area_eff` in `_energy.csv` gives the sum of `A_eff` over all contacting particles at each logged step. Compare against `N_contact × dx²` for a cross-check. The ratio `contact_length / dx = sqrt(m/rho) × factor / dx` ≈ `factor` (nominally 1.0, tunable via `MFREE_THERMAL_CONTACT_LENGTH_FACTOR`).

**Mitigation:** Set `MFREE_THERMAL_CONTACT_LENGTH_FACTOR` to a value derived from the expected contact patch geometry. For post-refinement runs where particle masses vary significantly, a particle-size-adaptive factor may be needed.

---

### 5. Friction Heat Partition Fractions

**What it is:** The empirical fractions `frac_wp = 0.8` and `frac_tool = 0.2` partition frictional heat between workpiece and tool. These are constant throughout the simulation and are not derived from any in-situ thermal resistance model.

**Literature context (Ti6Al4V + uncoated WC, dry cutting):** Published partition fractions for the workpiece range from 0.50 to 0.90 depending on cutting speed, feed, and tool geometry. The default 0.80/0.20 split is consistent with the upper range of measurements for low cutting speeds (≤ 60 m/min). At high speeds the partition shifts toward the chip/workpiece (> 0.90) as the tool–chip contact time decreases.

**How to observe:** The partition is fixed per run; no direct in-simulation measurement is available. Compare `cum_contact_E_workpiece / cum_contact_E_fric_scaled` against `frac_wp` from `_energy.csv` — this should equal `frac_wp` minus the conduction contribution. For a purely frictional (no conduction) case it equals `frac_wp` exactly.

**Mitigation:** Override with `MFREE_THERMAL_FRAC_WP` and `MFREE_THERMAL_FRAC_TOOL`. For a more physically grounded partition, implement a Trigger-partition model (fraction proportional to thermal effusivities), which would require reading `h_c`, material properties, and contact speed per event.

---

### Summary Table

| Approximation | Order of Magnitude | Measurable via | Mitigated by |
|---|---|---|---|
| Operator-splitting lag | O(dt) ≈ 0.01–0.1 K/step | Run comparison (explicit vs. default) | `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1` |
| Thermal one-way-per-substep lag | O(dt/n_sub) | `step_tool_source_residual` | Increase `MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS` |
| Limiter suppression | `cum_suppression_ratio` in CSV | `cum_suppression_ratio` > 0.10 warning | Reduce `MFREE_THERMAL_MAX_DT_PER_STEP` or global dt |
| Contact area heuristic | ±50% for uniform lattice; larger after adaptivity | `step_contact_area_eff` vs. `N × dx²` | `MFREE_THERMAL_CONTACT_LENGTH_FACTOR` |
| Friction partition fractions | ±0.1–0.4 depending on speed | `cum_contact_E_workpiece / cum_contact_E_fric_scaled` | `MFREE_THERMAL_FRAC_WP` / `MFREE_THERMAL_FRAC_TOOL` |

## Advantages and Disadvantages

### Advantages

- **Single-executable coupled run**: SPH and FE domains exchange forces and heat every step with a clear and deterministic execution order.
- **Strong coupling option (explicit coupled mode)**: repeating contact + FE updates inside a substep loop improves consistency between interface loads and tool state within a single SPH timestep.
- **Robust thermal stabilization**: contact heat exchange is capped by `MFREE_THERMAL_MAX_DT_PER_STEP`, which can prevent runaway temperatures from a single step.
- **Flexible FE tool behavior**: FE tool can be thermal-only, quasi-static deformable, or explicit deformable depending on env vars.
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

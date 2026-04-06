# Design Note: Adaptive Timestep & FE Tool Mass Scaling (No Behavior Change Yet)

This note proposes environment variables and precise code insertion points to add:

- Global adaptive timestep control (primarily SPH-driven).
- Optional FE tool mechanics mass scaling to reduce the required explicit subcycling.

Nothing in this document is implemented by itself; it is a design blueprint to implement later without changing default behavior.

## 1) Current State (Baseline)

- There is a single global timestep `dt` stored in `simulation_time` and used by the SPH integrator in [leap_frog::step](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/leap_frog.cpp#L108-L159).
- FE tool thermal uses the same `dt` via [body::advance_fe_tool_thermal](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L289-L294).
- FE tool mechanics estimates a critical timestep and warns if `dt` is too large: [fe_tool::mechanics_dt_crit](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L1176-L1213).
- When `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1`, FE tool mechanics is subcycled inside the contact iteration loop: [body::apply_contact](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L176-L257).

## 2) Adaptive Global Timestep (SPH-Driven)

### 2.1 Goals

- Choose `dt` each step based on stability constraints (CFL / wave speed / contact / thermal explicit limits).
- Preserve default behavior (fixed dt) unless explicitly enabled.

### 2.2 Proposed Environment Variables

- `MFREE_DT_ADAPTIVE=0/1`  
  Default `0` keeps fixed dt behavior.

- `MFREE_DT_SAFETY=0.9`  
  A global multiplier applied to the minimum of all dt limits.

- `MFREE_DT_MIN` (seconds)  
  Hard floor to prevent dt collapse (optional; if not set, no floor).

- `MFREE_DT_MAX` (seconds)  
  Hard cap (optional).

**SPH mechanics limiter**
- `MFREE_DT_CFL=0.25`  
  CFL coefficient for SPH (definition depends on how you compute it; typical form `dt <= CFL * h / (c0 + |v|)`).

**Contact limiter (optional)**
- `MFREE_DT_CONTACT_ENABLE=0/1` (default `0`)  
  Enables a contact-specific limiter based on penalty stiffness or observed penetration/impulse.
- `MFREE_DT_CONTACT_SAFETY=0.9`

**Thermal limiter (optional)**
- `MFREE_DT_THERMAL_ENABLE=0/1` (default `0`)  
  Enables an explicit thermal stability limiter for the FE tool and/or SPH thermal conduction.
- `MFREE_DT_THERMAL_SAFETY=0.5`

### 2.3 Where to Slot It

**Primary slot:** `simulation_time`  
- Add a method like `update_dt(body&)` that computes dt limits and calls `set_dt(new_dt)` internally when adaptive mode is enabled.

**Call site options:**
- Option A (simplest): at the start of each step in [leap_frog::step](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/leap_frog.cpp#L108-L125), before any predictor/corrector work.
- Option B (more robust): right after neighbor construction in the same function, because some dt estimates may depend on local spacing or max velocity.

**Data sources:**
- SPH: max |v|, smoothing length h, wave speed (from physical constants) can be obtained from `body` / `simulation_data`.
- Contact: penalty parameters from `contact.cpp` (would require exposing a dt limiter function or re-deriving a conservative bound).
- FE tool: `fe_tool::mechanics_dt_crit()` is already available and can be folded into a dt bound if desired.

### 2.4 Backward Compatibility

- Default: `MFREE_DT_ADAPTIVE=0` so dt remains fixed, identical behavior.
- Adaptive mode should only change `simulation_time::dt` when enabled.

## 3) FE Tool Mechanics Mass Scaling (Optional)

### 3.1 Goals

- Increase FE tool stable explicit timestep (`dtcrit`) by increasing the lumped mechanical mass (density-like scaling).
- Cap added mass to a user-controlled fraction to avoid overly nonphysical inertia.
- Preserve default behavior unless explicitly enabled.

### 3.2 Proposed Environment Variables

- `MFREE_FE_TOOL_MASS_SCALING_ENABLE=0/1` (default `0`)
- `MFREE_FE_TOOL_MASS_SCALING_MAX_FRAC=0.05`  
  Maximum allowed added mass fraction relative to original lumped mass.
- `MFREE_FE_TOOL_DT_TARGET` (seconds, optional)  
  If set, scale mass so that `0.9*dtcrit >= dt_target` if possible within the max mass fraction.
- `MFREE_FE_TOOL_MASS_SCALING_MODE=dt_target|substeps_target` (default `dt_target`)
- `MFREE_FE_TOOL_SUBSTEPS_TARGET` (optional)  
  Alternative target: choose mass scaling to keep FE substeps ≤ this number for the current global dt.

### 3.3 Where to Slot It

**Primary slot:** FE tool mechanical mass assembly  
- Implement scaling inside [fe_tool::ensure_mechanics_lumped_mass](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L1143-L1174) immediately after computing the baseline lumped mass vector.

**Control logic input:**
- Current global `dt` can be passed in or pulled from `simulation_time`.
- Current `dtcrit` is computed by [fe_tool::mechanics_dt_crit](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/fe_tool.cpp#L1176-L1213).

**Effect on coupling:**
- Mass scaling reduces required FE subcycling in [body::apply_contact](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L176-L257) because `dtcrit` increases.
- This would allow larger `dt_sub` without changing SPH dt.

### 3.4 Scaling Rule (Suggested)

Using a wave-speed estimate \(c \propto \sqrt{E/\\rho}\), the critical timestep roughly scales like:

\[
dt_{crit} \\propto \\sqrt{\\rho}
\]

So to increase \(dt_{crit}\) by a factor \(r = dt_{target}/dt_{crit}\), you need:

\[
\\rho_{new} = r^2 \\rho_{old}
\]

Apply this as a multiplier to the lumped mass vector \(M\) (not necessarily to thermal capacity).

Clamp the scaling so that added mass fraction ≤ `MFREE_FE_TOOL_MASS_SCALING_MAX_FRAC`.

### 3.5 Backward Compatibility

- Default: `MFREE_FE_TOOL_MASS_SCALING_ENABLE=0` → no change to mass.
- If enabled, scaling should be deterministic and logged once (warn if capped).

## 4) Multi-Rate Time Stepping (FE dt ≠ SPH dt)

### 4.1 What Already Exists

- FE mechanics subcycling within a global SPH dt in the deformable explicit coupled contact loop:
  - `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1`
  - auto substep count computed from `dt / (0.9*dtcrit)` or forced by `MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS`
  - see [body::apply_contact](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L176-L257)

### 4.2 Proposed Enhancement (Later)

- Allow FE thermal subcycling as well:
  - `MFREE_FE_TOOL_THERMAL_SUBSTEPS` (default 1)
  - Slot: [body::advance_fe_tool_thermal](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/body.cpp#L289-L294) would call `advance_explicit(dt/substeps)` in a loop.

- Allow SPH subcycling (more invasive):
  - This would require restructuring `leap_frog::step` and the derivative accumulation pipeline; not recommended as the next incremental change.

## 5) Suggested Implementation Order (Future Work)

1) Add global adaptive dt plumbing in `simulation_time` behind `MFREE_DT_ADAPTIVE=1`.
2) Add FE tool mass scaling in `fe_tool::ensure_mechanics_lumped_mass` behind `MFREE_FE_TOOL_MASS_SCALING_ENABLE=1`.
3) Add FE thermal subcycling option (least invasive multi-rate extension).


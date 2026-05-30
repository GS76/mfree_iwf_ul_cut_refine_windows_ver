# Workpiece Convection Boundary Conditions (Thermal Model)

## Table of Contents

- [1. Scope](#1-scope)
- [2. Summary](#2-summary)
- [3. Implementation](#3-implementation)
- [4. Parameters](#4-parameters)
- [5. Applied Convection Properties](#5-applied-convection-properties)
- [6. Verification Checks](#6-verification-checks)
- [7. Findings and Gaps](#7-findings-and-gaps)

## 1. Scope

This document analyzes and documents all heat transfer *convection* properties currently applied to the *workpiece* in the solver’s thermal model.

The implementation is not a face-based “convection boundary condition” in the classical FEM sense (top/bottom/left/right). Instead, convection is implemented as a *global particle-wise sink term* applied to **all particles** when enabled.

## 2. Summary

- **Number of convection definitions:** 1 (global).
- **When it is active:** cooldown stage only (enabled at the end of cutting if cooldown mode is selected).
- **Convection coefficient:** default `h = 25.0 W/m^2/K` (CLI override).
- **Ambient fluid temperature:** `T∞ = 295.15 K` (hardcoded as 22 °C).
- **Surface assignment:** no explicit surface selection; applied to the entire workpiece body (all particles).
- **Stabilization:** global ramp factor to cap maximum predicted cooling rate.

## 3. Implementation

### 3.1 Where convection is computed

- Convection term: [thermal.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/thermal.cpp#L165-L213)
- Called as part of thermal solve: [thermal.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/thermal.cpp#L246-L259)

### 3.2 Activation and configuration

Convection is enabled only at the start of the cooldown phase in the main program:

- Cooldown hook: [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L308-L316)

Relevant CLI inputs:

- `--cooldown` (enables cooldown stage)
- `--cooldown-hconv <W/m^2/K>` (sets the convection coefficient; also enables cooldown)

## 4. Parameters

### 4.1 Convection parameters

- Heat transfer coefficient: `h` in `W/m^2/K`
- Ambient fluid temperature: `T∞` in `K`
- Optional rate cap: `max_rate` in `K/s`

Defaults currently in the program logic:

- `h = 25.0 W/m^2/K` (default; CLI override available): [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L112-L141)
- `T∞ = 273.15 + 22.0 = 295.15 K` (hardcoded): [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L238-L244)
- `max_rate = 5.0/60.0 K/s` (cooling-rate cap; hardcoded): [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L241-L244)

### 4.2 Particle geometry proxy used for convection

The code does not compute actual exposed surface area for the workpiece. It computes an *effective* area-to-volume ratio per particle using:

- Effective volume per particle:

$$
dV_i = \frac{m_i}{\rho_0(T_i)}
$$

- Effective area-to-volume ratio:

$$
\left(\frac{A}{V}\right)_i = \frac{4}{\sqrt{dV_i}}
$$

These appear in [thermal.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/thermal.cpp#L182-L186).

## 5. Applied Convection Properties

### 5.1 BC #1 — Cooldown convection (global / particle-wise)

**Identification**

- **Name:** Cooldown convection (global)
- **Enabled by:** cooldown stage only
- **Implementation:** [thermal.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/thermal.cpp#L165-L213)

**Mathematical form (as coded)**

For each particle $i$:

$$
\beta_i = \frac{h\,(A/V)_i}{\rho_0(T_i)\,c_p(T_i)}
$$

$$
\dot{T}_i \mathrel{+}= -r \,\beta_i \,(T_i - T_\infty)
$$

where:

- $h$ is the convection coefficient (`m_h_W_m2K`)
- $T_\infty$ is ambient fluid temperature (`m_T_ambient_K`)
- $c_p(T)$ is the material heat capacity from the thermal constants table
- $r$ is a global ramp factor applied when a cooling-rate cap is active

**Cooling-rate ramp (stabilization)**

The solver estimates the maximum predicted convection rate across all particles,

$$
\max_i \left|\beta_i (T_i - T_\infty)\right|
$$

and applies:

$$
r=
\begin{cases}
1, & \text{if } \max\_rate \le 0 \text{ or } \max_i|\beta_i (T_i - T_\infty)| \le \max\_rate, \\
\frac{\max\_rate}{\max_i|\beta_i (T_i - T_\infty)|}, & \text{otherwise}.
\end{cases}
$$

This behavior is implemented in [thermal.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/thermal.cpp#L191-L197).

**Surface designation and affected area**

- **Surface designation:** none (no “top/bottom/side” mapping).
- **Affected region:** all workpiece particles (including interior), because the convection loop iterates over `i = 0..num_part-1` without boundary tests: [thermal.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/thermal.cpp#L173-L212)
- **Area model:** implicit per particle via $(A/V)_i = 4/\sqrt{dV_i}$

**Coefficient and ambient temperature**

| Property | Value | Source |
|---|---:|---|
| Convection coefficient $h$ | 25.0 W/m²·K (default) | [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L112-L141) |
| Ambient temperature $T_\infty$ | 295.15 K (22 °C) | [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L238-L244) |
| Cooling cap $\max\_rate$ | 0.08333 K/s (5 °C/min) | [refine_cut_main.cpp](file:///d:/mfree_iwf_ul_cut_refine_windows_ver/src/refine_cut_main.cpp#L241-L244) |

**Flow characteristics (velocity/turbulence)**

- Not modeled. The code uses a single scalar $h$ without dependence on flow velocity, Reynolds number, turbulence model, or orientation.

## 6. Verification Checks

### 6.1 Correct assignment to workpiece surfaces

- The current implementation does not provide surface-based assignment. It applies convection to **all** particles.
- If the intended operating condition is “convection only on exposed external boundaries,” then the current implementation does not match that intent.

### 6.2 Missing convection definitions

- No per-surface convection definitions exist (e.g., separate $h$ for top surface vs sides).
- No mechanism exists to exclude tool-contact regions or fixed boundaries from convection.

### 6.3 Duplicate convection definitions

- No duplicates found. There is a single convection implementation and a single runtime configuration point.

### 6.4 Parameter consistency with operating conditions

- Default $h=25$ W/m²·K is within typical natural convection orders of magnitude for air, but may be inappropriate for forced convection or coolant flows.
- $T_\infty$ is hardcoded (22 °C), which may not match the intended environment without modifying the source.

## 7. Findings and Gaps

- The solver’s “convection” behaves as a global particle-wise Newton cooling sink.
- There is no explicit surface selection; “surface area” is approximated through a per-particle $(A/V)_i$ proxy.
- Convection is not configurable through JSON config; it is enabled and parameterized during cooldown via CLI flags and hardcoded constants.

### Code snippets (for context)

```cpp
// src/thermal.cpp (apply_convection)
const double dV = particles[i].m / rho0;
const double A_over_V = 4.0 / std::sqrt(dV);
const double beta = m_h_W_m2K * A_over_V / (rho0 * cp);
particles[i].T_t += -ramp * beta * (Ti - m_T_ambient_K);
```

```cpp
// src/refine_cut_main.cpp (cooldown start)
trml->set_convection(cooldown_hconv_W_m2K, ambient_T_K);
trml->set_max_cooling_rate(max_rate_K_per_s);
trml->set_convection_enabled(true);
```

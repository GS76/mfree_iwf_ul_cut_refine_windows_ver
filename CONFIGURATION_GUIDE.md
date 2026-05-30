# Configuration Guide

This guide documents the user-configurable parameters for the `mfree_iwf_ul_cut_refine` simulation. The recommended workflow is to use a runtime JSON configuration file so input changes do not require recompilation.

## 1. Command Line Interface

### 1.1 Config-driven runs (recommended)

*   **Run from config**: `.\build\Release\mfree_iwf.exe --config configs\model1.json`
*   **Dump a complete default config**: `.\build\Release\mfree_iwf.exe --dump-config configs\default.json`
*   **Schema reference**: see [CONFIG_SCHEMA.md](CONFIG_SCHEMA.md)

The simulation executable accepts a single argument to select the benchmark model.

*   **Flag**: `-m [int]`
*   **Description**: Selects the simulation scenario.
*   **Valid Values**:
    *   `1`: Single-resolution cutting (Reference).
    *   `2`: Dynamic multi-resolution (Adaptive).
    *   `3`: A-priori multi-resolution (Static refinement).
    *   `4`: Same as 1.
*   **Default**: `1`
*   **Example**: `.\build\Release\mfree_iwf.exe -m 2`

## 2. Simulation Configuration

These settings control the simulation duration, time step, and output frequency.

| Parameter | Location | Description | Default / Calculation |
| :--- | :--- | :--- | :--- |
| `num_print` | `src/refine_cut_main.cpp` (L152) | Number of output frames to generate. | `150` |
| `t_final` | `src/benchmarks/test_cuttings.cpp` | Total physical time to simulate. | Calculated based on cut length (e.g., 1mm) and cutting speed. |
| `dt` | `src/benchmarks/test_cuttings.cpp` | Time step size. | Calculated dynamically based on CFL condition (min of mechanical and thermal CFL). |

**How to Change**:
*   Prefer `--config` and edit the JSON file.
*   Use `--dump-config` to generate a complete baseline JSON that can be committed and edited.

## 3. Material Parameters

Material properties are defined in `src/benchmarks/material_library.cpp`. The default material for Models 1-4 is **Ti6Al4V** (Sima 2010 modification).

### 3.1 Physical Constants
Defined in `matlib_tial6v4_Sima_tanh2010_SI`.

| Parameter | Variable | Value (Ti6Al4V) | Description |
| :--- | :--- | :--- | :--- |
| Young's Modulus | `E` | `113.8 GPa` | Stiffness of the material. |
| Poisson's Ratio | `nu` | `0.35` | Ratio of transverse to axial strain. |
| Density | `rho0` | `4430 kg/m³` | Initial density. |

### 3.2 Johnson-Cook Plasticity
Parameters for the flow stress equation: $\sigma = (A + B\varepsilon^n)(1 + C \ln \dot{\varepsilon}^*)(1 - T^{*m})$ + Sima tanh terms.

| Parameter | Variable | Value | Description |
| :--- | :--- | :--- | :--- |
| Yield Stress | `JC_A` | `724.7 MPa` | Initial yield stress. |
| Hardening Coeff | `JC_B` | `683.1 MPa` | Strain hardening coefficient. |
| Hardening Exp | `JC_n` | `0.47` | Strain hardening exponent. |
| Strain Rate Coeff | `JC_C` | `0.035` | Strain rate sensitivity. |
| Thermal Softening | `JC_m` | `1.0` | Thermal softening exponent. |
| Melting Temp | `Tmelt` | `1878 K` | Material melting point. |
| Ref Temp | `Tref` | `298 K` | Reference (room) temperature. |

**How to Change**:
*   Prefer `--config` and set `material.physical_constants` and `plasticity.model`.

## 4. Numerical Parameters

Constants for numerical stabilization and correction methods, defined in `src/benchmarks/test_cuttings.cpp`.

| Parameter | Variable | Default | Description |
| :--- | :--- | :--- | :--- |
| **Artificial Viscosity** | | | |
| Alpha | `alpha` | `1.0` | Linear viscosity coefficient (Monaghan). |
| Beta | `beta` | `1.0` | Quadratic viscosity coefficient (Monaghan). |
| Eta | `eta` | `0.1` | Smoothing parameter to prevent singularity. |
| **Stabilization** | | | |
| XSPH Epsilon | `xsph_eps` | `0.5` | Velocity smoothing factor (0.0 - 1.0). |
| **Stress Correction** | | | |
| Monaghan Eps | `art_stress_eps` | `0.3` | Artificial stress coefficient to prevent tensile instability. |
| Exponent | `stress_exponent` | `4.0` | Exponent for stress correction repulsion. |

## 5. Geometry and Tool Settings

Defined in `src/benchmarks/test_cuttings.cpp` (functions `cutting_ref_*`).

| Parameter | Variable | Default (Model 1) | Description |
| :--- | :--- | :--- | :--- |
| **Workpiece** | | | |
| Dimensions | `lo_x`, `hi_x`, `lo_y`, `hi_y` | 0-2mm (x), 0.3-0.8mm (y) | Bounding box of the workpiece. |
| Resolution | `nx`, `ny` | Derived from `nbox` | Number of particles. |
| **Tool** | | | |
| Cutting Speed | `vc` | `8.33 m/s` (500 m/min) | Velocity of the tool. |
| Feed Rate | `target_feed` | `0.2 mm` | Depth of cut. |
| Rake Angle | `rake` | `-5 deg` | Tool rake angle. |
| Clearance | `clear` | `5 deg` | Tool clearance angle. |
| Friction | `mu_friction` | `0.35` | Coulomb friction coefficient. |
| Radius | `fillet_radius` | `50 µm` | Tool edge radius. |

**How to Change**:
*   Prefer `--config` and edit the JSON file (tool/workpiece dimensions, angles, fillet, feed, and offsets).

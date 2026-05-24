# FE Tool Temperature-Dependent Material (Linear Thermoelastic)

This document describes the implemented temperature-dependent material support for the FE tool, including thermal–mechanical coupling.

## Scope

- Kinematics: small strain, small rotation (plane strain, P1 triangles).
- Mechanics: linear isotropic thermoelasticity with temperature-dependent parameters.
- Thermal: diffusion with temperature-dependent conductivity and heat capacity.
- Coupling: contact maps force and power into the FE tool; FE solves thermal explicitly and (optionally) mechanics explicitly.

Not implemented here:
- Large strain/large rotation kinematics, objective stress rates.
- Nonlinear plasticity/damage constitutive models for the FE tool.
- Mortar/Lagrange-multiplier FE contact; SPH contact remains penalty/friction on a polygonal boundary.

## Governing Equations

### Thermal (tool)

Let \(T(\mathbf{x},t)\) be temperature, \(\rho(T)\) density, \(c_p(T)\) heat capacity, \(k(T)\) conductivity, and \(q\) volumetric heat source.

\[
\rho(T)c_p(T)\,\dot{T} - \nabla \cdot (k(T)\nabla T) = q
\]

### Mechanics (tool, plane strain, small strain)

Displacement \(\mathbf{u}\), strain \(\boldsymbol{\varepsilon} = \tfrac{1}{2}(\nabla \mathbf{u} + \nabla \mathbf{u}^T)\).

Thermal strain:
\[
\boldsymbol{\varepsilon}_{th}(T) = \alpha(T)\,(T - T_{ref})\,\mathbf{I}
\]

Stress:
\[
\boldsymbol{\sigma} = \mathbb{C}(T):(\boldsymbol{\varepsilon} - \boldsymbol{\varepsilon}_{th})
\]
with isotropic \(\mathbb{C}(T)\) defined by \(E(T),\nu(T)\).

Semi-discrete equation of motion (explicit mode):
\[
\mathbf{M}\,\ddot{\mathbf{u}} + \mathbf{C}\,\dot{\mathbf{u}} + \mathbf{K}(T)\,\mathbf{u} = \mathbf{f}_{ext} + \mathbf{f}_{th}(T)
\]

Rayleigh damping (optional):
\[
\mathbf{C} = a_0 \mathbf{M} + a_1 \mathbf{K}(T)
\]

## Discretization and Algorithms

### Thermal explicit step

- Assembles a temperature-dependent conduction operator and nodal capacities each step if any of \(\rho(T),c_p(T),k(T)\) tables are provided.
- Applies Dirichlet boundaries (fixed temperature) and skips updating fixed nodes.

Flow:
1. Apply Dirichlet and build fixed-node mask.
2. Build \(K_T(T)\), \(C_T(T)\) if tables are enabled.
3. Compute nodal power from conduction + convection + mapped sources.
4. Update \(T^{n+1} = T^n + \Delta t\,P/C_T\) for non-fixed nodes.

### Mechanics explicit step (optional)

- Uses a lumped (diagonal) mass vector \(M\) and a central-difference/leapfrog update with velocity at half steps.
- Rebuilds \(K(T)\) each step if \(E(T)\) or \(\nu(T)\) tables are provided.
- Adds thermoelastic equivalent nodal load based on \(\alpha(T)\), \(E(T)\), \(\nu(T)\) evaluated per element at \(T_{avg}\).

Flow:
1. Rebuild \(K(T)\) if needed.
2. Ensure lumped mass \(M(T)\) (uses \(\rho(T)\) if provided).
3. Assemble \(\mathbf{f} = \mathbf{f}_{contact} + \mathbf{f}_{th}(T)\).
4. Compute \(\mathbf{a} = M^{-1}(\mathbf{f} - K(T)\mathbf{u} - C\mathbf{v})\).
5. Update \(\mathbf{v}^{n+1/2} = \mathbf{v}^{n-1/2} + \Delta t\,\mathbf{a}\).
6. Update \(\mathbf{u}^{n+1} = \mathbf{u}^{n} + \Delta t\,\mathbf{v}^{n+1/2}\).

### Explicit Coupled Contact Subcycling (optional)

When enabled, the deformable contact loop can use explicit FE mechanics subcycling within the global timestep to better couple:

\[
\text{contact} \rightarrow \text{FE explicit mechanics} \rightarrow \text{updated boundary} \rightarrow \text{contact}
\]

Configuration:
- `MFREE_DEFORMABLE_FE_TOOL=1`
- `MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1`
- Optional: `MFREE_DEFORMABLE_TOOL_EXPLICIT_SUBSTEPS` (forces a fixed number of substeps)
- Optional: `MFREE_DEFORMABLE_TOOL_EXPLICIT_MAX_SUBSTEPS` (cap for auto-computed substeps)
- Optional: `MFREE_DEFORMABLE_TOOL_THERMAL_SUBSTEPS` (if set, uses a different substep count for FE thermal than for FE mechanics)

## Configuration (Environment Variables)

### Temperature tables

Tables are piecewise-linear and clamped outside the provided range. Format:

- `T1:V1,T2:V2,...` (commas/semicolons/whitespace allowed, `:` or `=` allowed).

Canonical naming contract:
- `MFREE_<DOMAIN>_<PROPERTY>_TABLE`
- `<DOMAIN>` is `FE_TOOL` for tool fields and `WP` for workpiece fields.
- All table readers now use the same parser contract and interpolation behavior.

Thermal:
- `MFREE_FE_TOOL_RHO_TABLE`
- `MFREE_FE_TOOL_CP_TABLE`
- `MFREE_FE_TOOL_K_TABLE`

Mechanical:
- `MFREE_FE_TOOL_E_TABLE`
- `MFREE_FE_TOOL_NU_TABLE`
- `MFREE_FE_TOOL_ALPHA_TABLE`

Supported FE alias names (equivalent to canonical FE tool names):
- `MFREE_FE_RHO_TABLE`, `MFREE_FE_CP_TABLE`, `MFREE_FE_K_TABLE`
- `MFREE_FE_E_TABLE`, `MFREE_FE_NU_TABLE`, `MFREE_FE_ALPHA_TABLE`

Workpiece temperature-table names (same format):
- Thermal: `MFREE_WP_K_TABLE`, `MFREE_WP_CP_TABLE`
- Mechanical: `MFREE_WP_E_TABLE`, `MFREE_WP_G_TABLE`

Supported workpiece alias names:
- `MFREE_WORKPIECE_K_TABLE`, `MFREE_WORKPIECE_CP_TABLE`
- `MFREE_WORKPIECE_E_TABLE`, `MFREE_WORKPIECE_G_TABLE`

### Mechanics explicit toggle and damping

- `MFREE_FE_TOOL_MECH_EXPLICIT=1` enables explicit mechanics update of the FE tool each global timestep.
- `MFREE_FE_TOOL_RAYLEIGH_A0` (≥ 0)
- `MFREE_FE_TOOL_RAYLEIGH_A1` (≥ 0)

## Verification Suggestions (Implemented Features)

- Thermal 1D diffusion (manufactured solution): verify convergence of \(T\) with decreasing \(\Delta t\).
- Thermoelastic expansion of a constrained bar: verify displacement scales with \(\alpha(T)\Delta T\).
- Temperature-dependent stiffness: apply a fixed nodal load and verify displacement changes with \(E(T)\).

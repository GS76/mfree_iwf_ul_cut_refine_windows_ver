# SPH Workpiece Temperature-Dependent Properties
This document describes environment-variable inputs for SPH workpiece temperature-dependent Poisson ratio and thermal expansion coefficient.

## Supported environment variables
- `MFREE_WP_NU_TABLE` (alias: `MFREE_WORKPIECE_NU_TABLE`)
- `MFREE_WP_ALPHA_TABLE` (alias: `MFREE_WORKPIECE_ALPHA_TABLE`)

Optional scalar overrides:
- `MFREE_WP_NU` (alias: `MFREE_WORKPIECE_NU`)
- `MFREE_WP_ALPHA` (alias: `MFREE_WORKPIECE_ALPHA`)

Table format:
- `T1:V1,T2:V2,...`
- `:` or `=` separators are accepted.
- Comma/semicolon/whitespace separated pairs are accepted.
- At least two points are required.
- Linear interpolation is used between points, clamped outside range.

## Where these are applied
- `nu(T)` is used in SPH constitutive updates and plastic radial return via temperature-dependent shear modulus.
- `alpha(T)` is used in SPH EOS as a thermal pressure correction term:
  - `p = c0^2 (rho-rho0) - 3 K alpha(T) (T-Tref)`

## Ti-6Al-4V starter values
Example starter tables for Ti-6Al-4V (adjust for your data source):
- `MFREE_WP_NU_TABLE="293:0.34, 800:0.35, 1200:0.36"`
- `MFREE_WP_ALPHA_TABLE="293:8.6e-6, 800:9.5e-6, 1200:1.05e-5"`

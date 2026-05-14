# Adaptive Penalty Contact (SPH ↔ Tool Boundary)

This document describes the implemented adaptive penalty parameter option for the existing SPH contact model.

## Model Summary

The contact enforcement remains penalty-based with Coulomb friction (no exact constraints).

For a particle of mass \(m_s\) penetrating a tool boundary by \(g_N\) with normal \(\mathbf{n}\), the normal penalty force is:

\[
\mathbf{f}_N = -\alpha \frac{m_s}{\Delta t^2} g_N \mathbf{n}
\]

and tangential friction is computed by the existing LDYNA-style update with yield cap \(\mu\|\mathbf{f}_N\|\).

## Adaptive Penalty Parameter

The code supports an optional adaptive scaling of \(\alpha\) based on current penetration magnitude:

\[
\alpha = \mathrm{clamp}\left(\alpha_0 \cdot \max\left(1, \frac{|g_N|}{g_{ref}}\right),\ \alpha_{min},\ \alpha_{max}\right)
\]

This is a simple heuristic intended to stiffen contact response when penetrations become large relative to a target penetration \(g_{ref}\).

## Configuration (Environment Variables)

- `MFREE_CONTACT_ALPHA` (default 0.1)
- `MFREE_CONTACT_ADAPTIVE_PENALTY` (0/1, default 0)
- `MFREE_CONTACT_PEN_DEPTH_REF` (default 1e-6)
- `MFREE_CONTACT_ALPHA_MIN` (default 1e-4)
- `MFREE_CONTACT_ALPHA_MAX` (default 10)

## Notes and Limitations

- This does not enforce non-penetration exactly; it only adjusts penalty strength.
- Too large \(\alpha\) can cause instability if the global timestep is not small enough.
- For exact constraints, a different formulation (e.g., Lagrange multipliers or augmented Lagrangian) is required.

## Lagrange Multiplier Option (Augmented Update)

When enabled, the contact code can accumulate a per-particle normal multiplier \(\lambda_N\) and compute the normal contact force as:

\[
\mathbf{f}_N = \lambda_N \mathbf{n}
\]

with an augmented update each step:

\[
\lambda_N \leftarrow \max(0,\ \lambda_N - \rho g_N)
\quad,\quad
\rho = \alpha \frac{m_s}{\Delta t^2}
\]

Configuration:
- `MFREE_CONTACT_USE_LM=1`

This is not a full mortar/Lagrange-multiplier FE contact solve; it is a per-particle augmented multiplier on top of the existing SPH contact detection.

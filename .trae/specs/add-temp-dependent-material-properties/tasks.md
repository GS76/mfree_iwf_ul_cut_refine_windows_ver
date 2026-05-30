# Tasks
- [x] Task 1: Add linear-fit support to temperature-dependent properties.
  - [x] Add a linear equation mode (`y = m*T + b`) to the property evaluation utility.
  - [x] Add min/max clamping options suitable for physical properties.
  - [x] Verify constant mode and table mode remain unchanged.

- [x] Task 2: Expose linear expansion and linear-fit properties in simulation constants.
  - [x] Add `α(T)` storage and accessor in physical constants.
  - [x] Add setters to configure linear fits for `ρ(T)`, `α(T)`, `k(T)`, `E(T)`, `ν(T)`, `Cp(T)`.
  - [x] Ensure call sites can query per-particle temperature without caching stale values.

- [x] Task 3: Update the material library to configure the provided equations.
  - [x] Set `ρ(T)`, `α(T)`, `k(T)`, `E(T)`, `ν(T)`, `Cp(T)` using the specified linear fits.
  - [x] Add any necessary clamps/ranges to keep values physical across expected temperatures.

- [x] Task 4: Integrate temperature-dependent properties in thermal and structural updates.
  - [x] Ensure thermal conduction uses `ρ(T)`, `k(T)`, and `Cp(T)` at current temperature.
  - [x] Ensure constitutive response uses `E(T)` and `ν(T)` at current temperature.
  - [x] Wire `α(T)` into the solver where thermal expansion is required by the model (or expose it for downstream use).

- [x] Task 5: Add and run verification.
  - [x] Extend or add unit tests to cover linear-fit property evaluation and clamping.
  - [x] Add a regression-style test that constructs the material constants and spot-checks values at representative temperatures.

# Task Dependencies
- Task 2 depends on Task 1
- Task 3 depends on Task 2
- Task 4 depends on Task 2 and Task 3
- Task 5 depends on Task 1–4

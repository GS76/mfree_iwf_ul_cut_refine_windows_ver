# Temperature-Dependent Material Properties Spec

## Why
The thermal–structural solver operates over a wide temperature range, so material properties must change with the computed temperature to remain physically meaningful and numerically stable.

## What Changes
- Add first-class support for temperature-dependent properties defined by linear fits of the form `y = m*T + b`.
- Populate the solver’s material model with the provided linear equations for:
  - density `ρ(T)`, linear expansion `α(T)`, thermal conductivity `k(T)`, Young’s modulus `E(T)`, Poisson’s ratio `ν(T)`, specific heat `Cp(T)`.
- Ensure thermal and structural updates query properties using each particle’s current temperature.
- Add validation/guards to prevent non-physical property values from destabilizing the simulation.

## Impact
- Affected specs: thermal conduction, equation of state / elasticity, material library initialization, unit tests
- Affected code:
  - Property evaluation utilities: `src/property.h`
  - Constants storage and access: `src/simulation_data.h`, `src/simulation_data.cpp`
  - Thermal conduction: `src/thermal.cpp`
  - Structural constitutive update/EOS: `src/material.cpp`
  - Material presets: `src/benchmarks/material_library.cpp`
  - Tests: `src/tests/test_property_interpolation.cpp` (and/or new tests)

## ADDED Requirements

### Requirement: Linear temperature-dependent property definition
The system SHALL support defining a material property via a linear equation `y = m*T + b` and evaluating it at runtime using the temperature field `T` used by the simulation.

#### Scenario: Evaluate linear property
- **WHEN** a property is configured with coefficients `(m, b)` and queried at temperature `T`
- **THEN** the returned value equals `m*T + b` (within floating-point tolerance)

#### Scenario: Safe evaluation bounds
- **WHEN** a linear property is evaluated at a temperature that would produce a non-physical value (e.g., negative density or negative conductivity)
- **THEN** the evaluation SHALL be clamped to a configured minimum (and, where applicable, maximum) to keep the simulation stable

### Requirement: Temperature-dependent physical and thermal constants
The system SHALL expose temperature-dependent accessors for `ρ(T)`, `α(T)`, `k(T)`, `E(T)`, `ν(T)`, and `Cp(T)` through the existing constants objects used by the solver.

#### Scenario: Query by particle temperature
- **WHEN** the solver updates thermal conduction and constitutive response for a particle with temperature `T`
- **THEN** it uses `ρ(T)`, `k(T)`, `Cp(T)`, `E(T)`, and `ν(T)` evaluated at that same `T`

### Requirement: Material preset uses provided linear equations
The system SHALL configure the default/benchmark material preset to use the following linear equations (with `T` in the simulation’s temperature units):
- `ρ(T)  = -0.1401*T + 4464.74`
- `α(T)  = 1.08E-05*T - 3.41E-03`
- `k(T)  = 0.0178*T + 0.394`
- `E(T)  = -5.36E+07*T + 1.32E+11`
- `ν(T)  = 4.15E-05*T + 0.3053`
- `Cp(T) = 0.2273*T + 491.62`

#### Scenario: Initialization
- **WHEN** the material library creates the material constants for the target material
- **THEN** the constants are configured so that each of the above properties evaluates per the provided equation

## MODIFIED Requirements

### Requirement: Temperature dependence for existing table-based properties
Existing table-based temperature-dependent properties SHALL continue to work, and the new linear-fit mode SHALL not break existing constant or table behavior.

## REMOVED Requirements
None.

# Temperature-Dependent Property Variation Check Spec

## Why
Temperature-dependent material properties are only useful if they demonstrably change with temperature in the configured range and are actually queried by the solver at runtime. A clear verification procedure prevents “silent constant-property” regressions.

## What Changes
- Define the best-practice verification methods to confirm equation-based properties vary with temperature.
- Add an automated verification test strategy (unit-level and configuration-level) that can be run in CI via CTest.
- Define optional lightweight diagnostic outputs for manual inspection (CSV/property table), without requiring a full cutting run.

## Impact
- Affected specs: temperature-dependent property support, validation suite, regression testing
- Affected code (expected):
  - Property evaluation: `src/property.h`
  - Material preset setup: `src/benchmarks/material_library.cpp`
  - Tests: `src/tests/test_property_interpolation.cpp` (or new tests)
  - Optional diagnostics: `src/refine_cut_main.cpp` (if a CLI “dump properties” mode is added)

## ADDED Requirements

### Requirement: Automated variation verification
The validation suite SHALL include a test that proves each equation-based property varies with temperature over a defined sampling range.

#### Scenario: Linear equation varies with temperature
- **WHEN** a linear-fit property `y(T)=m*T+b` is configured with `m != 0`
- **THEN** sampling the property at two distinct temperatures `T1 != T2` SHALL produce different values
- **AND** the test SHALL assert the delta matches the expected `m*(T2-T1)` within tolerance.

#### Scenario: Clamp behavior is detectable and correct
- **WHEN** a linear-fit property is configured with a clamp (min and/or max)
- **THEN** evaluating at a temperature that would exceed the clamp SHALL return exactly the clamp value
- **AND** evaluating within range SHALL return the unclamped linear value.

### Requirement: Preset configuration verification
The validation suite SHALL include a test that constructs the material preset and verifies the configured equations produce distinct values across temperature.

#### Scenario: Material preset spot-check
- **WHEN** the default/benchmark material preset is constructed
- **THEN** each of `ρ(T)`, `α(T)`, `k(T)`, `E(T)`, `ν(T)`, `Cp(T)` SHALL be evaluated at representative temperatures (e.g., `Tref`, `Tref+200K`, `Tref+600K`)
- **AND** each property SHALL show temperature variation consistent with its configured equation and clamps.

### Requirement: Manual inspection pathway (optional)
The solver SHALL provide a manual workflow to inspect property variation without running a full simulation.

#### Scenario: Dump property table (optional CLI mode)
- **WHEN** the user runs a “dump properties” mode with a temperature range and step
- **THEN** the solver writes a CSV containing sampled values of each configured property vs. temperature
- **AND** the CSV uses explicit units and column naming to avoid ambiguity.

## MODIFIED Requirements
None.

## REMOVED Requirements
None.

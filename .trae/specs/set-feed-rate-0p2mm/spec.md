# Feed Rate 0.2 mm Spec

## Why
The cutting benchmark configuration currently targets a feed rate of 0.1 mm. We need to run benchmarks at 0.2 mm feed to match the desired process condition.

## What Changes
- Update the benchmark “target feed” value from 0.1 mm to 0.2 mm in the SI cutting setups.
- Ensure the printed `feed:` line reflects the updated target.
- Verify the smoke test still runs and produces outputs.

## Impact
- Affected specs: benchmark scenario configuration
- Affected code:
  - `src/benchmarks/test_cuttings.cpp` (hard-coded `target_feed`)
  - `src/refine_cut_main.cpp` (model selection calls the benchmark setup)
  - CTest `smoke_model_1` (sanity verification)

## ADDED Requirements

### Requirement: Benchmark feed rate is 0.2 mm
The benchmark scenario configuration SHALL target a feed rate of 0.2 mm (SI units: 2e-4 m).

#### Scenario: Model 1 feed configuration
- **WHEN** Model 1 benchmark is initialized
- **THEN** the tool pre-positioning logic uses `target_feed = 2e-4` meters
- **AND** the startup log prints `feed:` close to 0.000200 (within numerical tolerance).

#### Scenario: Model 2/3 feed configuration (if enabled)
- **WHEN** multi-resolution benchmark models are initialized
- **THEN** their `target_feed` settings use 0.2 mm as well.

## MODIFIED Requirements
None.

## REMOVED Requirements
None.

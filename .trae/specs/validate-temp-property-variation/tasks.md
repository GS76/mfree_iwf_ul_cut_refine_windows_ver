# Tasks
- [ ] Task 1: Add/extend unit tests to prove property variation with temperature.
  - [ ] Add assertions that linear-fit properties produce different values at different temperatures.
  - [ ] Add assertions that clamps apply at out-of-range temperatures and do not apply in-range.

- [ ] Task 2: Add regression test for benchmark material preset configuration.
  - [ ] Construct the preset (material library) used by benchmarks.
  - [ ] Evaluate `ρ, α, k, E, ν, Cp` at multiple representative temperatures.
  - [ ] Assert expected deltas and clamp behavior with tolerances.

- [ ] Task 3 (Optional): Add a non-simulation “dump properties” CLI mode.
  - [ ] Add CLI parsing for `--dump-properties --Tmin <K> --Tmax <K> --dT <K>`.
  - [ ] Write a CSV to `results/` that lists each property vs. temperature.
  - [ ] Document the intended usage in the test output or help text.

# Task Dependencies
- Task 2 depends on Task 1
- Task 3 depends on Task 2

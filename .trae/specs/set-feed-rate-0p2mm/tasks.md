# Tasks
- [x] Task 1: Update benchmark target feed to 0.2 mm.
  - [x] Change Model 1/4 `target_feed` to `2e-4` meters.
  - [x] Change multi-resolution model `target_feed` definitions to `2e-4` meters for consistency.

- [x] Task 2: Verify the updated feed configuration.
  - [x] Build `mfree_iwf`.
  - [x] Run `ctest -R smoke_model_1` and confirm it passes.
  - [x] Confirm the log prints `feed:` close to 0.000200.

# Task Dependencies
- Task 2 depends on Task 1

# Work Log

> Legacy historical log content is preserved for traceability and is not maintained as active forward-path guidance.
- 2026-06-01: external simulation execution guardrails enforced across scripts and campaign tooling
  - Completed
    - Added Warp-session execution guard to simulation PowerShell scripts:
      - `scripts/run_model5_fe_tool.ps1`
      - `scripts/run_steady_state_100m_min.ps1`
      - `scripts/run_200k_thermal_fix_test.ps1`
      - `scripts/check_timestep_100m_min.ps1`
    - Updated `scripts/campaign_mvp.py` `execute-manifest` to block non-dry runs inside Warp by default.
    - Added explicit override path for intentional in-Warp execution:
      - env var `MFREE_ALLOW_WARP_SIM=1`
      - CLI flag `--allow-in-warp` for `execute-manifest`
    - Updated user guidance in `docs/campaign_mvp_workflow.md` and `WARP.md` to match enforced behavior.
- 2026-06-01: Model 5 MVP campaign executed successfully and archived
  - Completed
    - Ran full MVP campaign from `config/campaign_model5_mvp.json` using `scripts/campaign_mvp.py` workflow.
    - Final manifest status in `results/campaigns/model5_mvp_phase1/artifacts/manifest.json`: `completed = 18` runs, `failed = 0`.
    - Normalized campaign dataset produced at `results/campaigns/model5_mvp_phase1/artifacts/normalized_results.csv`.
    - Constraint verification result: `constraint_suppression_ratio_limit_satisfied = 18 / 18 (100%)`.
    - Objective summary from normalized results:
      - `objective_cum_contact_E_tool_frac`: min `0.2806864281456547`, max `0.3230910453474268`
      - `objective_closure_residual_pct`: min `100.002343344018`, max `100.0023512564761`
    - Archived snapshot created at `results/baseline/20260601-1810/model5_mvp_phase1_campaign_success`.
  - Notes
    - Archive operation was performed as a copy, preserving the live campaign workspace under `results/campaigns/model5_mvp_phase1`.
- 2026-05-31: model 5 run parameters tuned for localized-heating investigation
  - Completed
    - Updated `scripts/run_model5_fe_tool.ps1` defaults to improve hotspot investigation fidelity:
      - denser output sampling (`OutputFrames=300`)
      - tighter moving-frame focus (`RefineDepthFactor=1.0`, `RefineFrameWidthMm=0.40`, `RefineHaloLayers=0`)
      - tighter thermal limiter default (`ThermalMaxDtPerStepK=2.0`)
    - Recorded FE tool ↔ SPH thermal contact coefficient model and defaults used by the script:
      - pressure-dependent conductance: `h_c(p) = h_sep + (h_full - h_sep) * clamp(p / p_ref, 0, 1)`
      - `ThermalHFull = 1.0e6` W/m²·K
      - `ThermalHSep = 1.0e4` W/m²·K
      - `ThermalPRef = 1.0e9` Pa
      - resulting active heat-transfer coefficient range: `1.0e4` to `1.0e6` W/m²·K (depending on local contact pressure)
    - Added explicit thermal-study knobs to script parameters for DOE-style sweeps:
      - contact conductance (`ThermalHFull`, `ThermalHSep`, `ThermalPRef`)
      - friction heat partition (`ThermalFracWp`, `ThermalFracTool`)
      - thermal guards (`RhoPseFloorFrac`, `ThermalSkipFrac`)
      - optional per-step data logging (`LogEveryStepData`)
    - Added post-run hotspot summary from `cutting_thermal.csv` (`wp_Tmax`, `tool_Tmax`) with configurable warning threshold (`WorkpieceTempWarnK`).
- 2026-05-31: image extraction workflow updated to default to VTK dual-scalar-bar screenshots
  - Completed
    - Added layout-aware extraction profile `paraview_vtk_dual_bar` and made it the default via `config/image_extraction_requirements.json`.
    - Extended ROI handling to auto-detect the active simulation scene and exclude scalar-bar/UI-heavy bands for ParaView-style VTK screenshots.
    - Added layout metadata to extraction outputs (`layout.profile`, `layout.analysis_roi_box_px`, scene detection diagnostics) and propagated these fields into CSV flattening.
    - Added `--layout-profile` to `scripts/extract_results_image_info.py` and `-LayoutProfile` passthrough in `scripts/run_image_extraction.ps1`, with `legacy` retained as an explicit override.
    - Updated `docs/image_extraction_workflow.md` to document the new default behavior and legacy override examples.
  - Validation performed
    - Ran strict extraction on `Bugs/Temperature_Refined_200k.png` with the updated requirements contract.
    - Confirmed `images_with_missing_fields = 0`, scene detection succeeded, and debug artifacts were produced.

- 2026-04-30: branch `feat/tighten-fe-sph-thermal-energy-accounting` — Phases 1–5 completed

  **Phase 1** (commit `7f40d2f8`): Remove dead `cumulative_*` fields from `fe_tool::thermal_energy_accounting`.  Fields were reset each step and aliased rather than accumulated; canonical source is the logger `m_cum_*` members.

  **Phase 2** (commit `5ca530d8`): Full-system energy closure accounting.
  - `plasticity`: `do_radial_return` now returns per-step Taylor-Quinney dissipation `sum(delta_T * m * cp)` over all yielding particles.
  - `body`: stores and exposes `m_step_plastic_dissipation` per step.
  - `fe_tool`: adds `thermal_internal_energy_above_ref(T_ref)` for reference-temperature-relative energy.
  - `logger`: reads `MFREE_THERMAL_T_REF` (default 298.15 K); captures per-run initial-state baselines; accumulates `m_cum_plastic_dissipation`; writes 7 new `_energy.csv` columns including `closure_residual` and `closure_residual_pct`.
  - Closure identity: `cum_plastic + cum_fric_scaled = delta_wp + delta_tool + cum_convection + cum_suppressed`.
  - Duplicated energy block in logger refactored into `log_energy_block()` helper.

  **Phase 3** (commit `bd19a87f`): VTK initial-state fields + self-contained chip classifier.
  - `vtk_writer`: adds `initial_x`, `initial_y`, `initial_temperature` per-particle scalars (double precision).
  - `extract_final_chip`: uses `initial_y` to derive `top_y`/`spacing` without requiring `out_000000.vtk`; uses per-particle `T_init` for `dE`; reads `delta_tool_internal_E` directly from energy CSV last row; `find_energy_csv()` accepts any `*_energy.csv` or legacy name; full backward-compat fallback for pre-Phase-3 VTKs.

  **Phase 4** (commit `28b17740`): Replace trivially-zero interface residual with suppression ratio.
  - `logger`: `step_interface_balance_residual` = `(E_wp+E_tool) − E_fric_scaled` was always 0 (algebraic identity). Replaced with `step_suppression_ratio = E_suppressed / (|E_cond_raw| + E_fric_raw)` = `(1−scale)`, which is 0 when the limiter is inactive and non-trivially non-zero when it fires.
  - `validate_main`: `test_interface_suppression_ratio()` — Case A (dt=1e-9 s): ratio=0, scale=1; Case B (dt=1.0 s): ratio=0.9998, scale=0.000193; tool-source residual=0.

  **Phase 5** (this entry): Coupling approximation documentation + suppression warning.
  - `docs/coupling_thermal_mechanical.md`: new "Known Approximations and Error Sources" section with 5 named approximations, quantitative estimates, measurement recipes, and a summary table.
  - `logger`: one-time stderr warning when `step_suppression_ratio > 0.10`, reset per results folder.

  **Full-run baseline measurement — recorded 2026-04-30:**

  Configuration: model 3 (`cutting_ref_multi_resol_dynamic`), explicit FE-tool coupling,
  default 500 m/min cutting speed, `MFREE_T_FINAL_SCALE=0.05`, 12 000 steps at
  dt = 5×10⁻¹⁰ s (6 µs physical time, 50 µm tool advance), 4 135 SPH particles.
  Run command:
  ```
  MFREE_FE_TOOL_MSH=snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh \
  MFREE_USE_FE_TOOL_FOR_CONTACT=1 MFREE_DEFORMABLE_FE_TOOL=1 \
  MFREE_DEFORMABLE_FE_TOOL_EXPLICIT=1 MFREE_FE_TOOL_FIX_TAGS=114 \
  MFREE_FE_TOOL_RHO=14500 MFREE_FE_TOOL_CP=20 MFREE_FE_TOOL_K=80 \
  MFREE_FE_TOOL_E=6e11 MFREE_FE_TOOL_NU=0.22 MFREE_FE_TOOL_ALPHA=4.5e-6 \
  MFREE_T_FINAL_SCALE=0.05 MFREE_MAX_STEPS=100000 \
  MFREE_THERMAL_T_REF=298.15 MFREE_LOG_ENERGY=1 \
  MFREE_RESULTS_DIR=results/baseline_phase5 MFREE_CLEAN_RESULTS=1 \
  ./build/Release/mfree_iwf.exe -m 3
  ```

  Final-row values from `results/baseline_phase5/cutting_energy.csv` (step 12 000, t = 6.0×10⁻⁶ s):

  | Metric | Column | Measured value | Notes |
  |---|---|---|---|
  | Limiter suppression (cumulative) | `cum_suppression_ratio` | **0.000** | Limiter never fired; dt well within thermal stability |
  | Tool-source residual (cumulative) | `cum_tool_source_residual` | **0.000 J** | Power mapping to FE nodes is perfectly conservative |
  | Full-system closure residual % | `closure_residual_pct` | **0.021 %** | Energy balance closes to within 0.02% ✔ |
  | Plastic dissipation fraction | `cum_plastic / (cum_plastic + cum_fric_scaled)` | **99.76 %** | Taylor-Quinney dominates; interface friction = 0.24% |
  | Tool energy fraction | `delta_tool / (cum_plastic + cum_fric_scaled)` | **0.09 %** | Low due to very short run (50 µm cut) and low tool cp=20 J/kgK |

  Additional derived values:
  - Total thermal input: 9.139 J (plastic: 9.117 J + friction: 0.022 J)
  - `delta_wp_internal_E` = 9.130 J (99.90% of input — workpiece retains nearly all heat)
  - `delta_tool_internal_E` = 8.23×10⁻³ J
  - `cum_tool_E_convection` = −8.28×10⁻⁴ J (small convective loss from tool surface)
  - `cum_contact_E_cond_raw` = 4.59×10⁻³ J (conduction from hot workpiece to cooler tool)

  Interpretation notes:
  - The 0.09% tool fraction is lower than the literature range (10–15% for WC + Ti6Al4V dry cutting).
    Two contributing factors: (a) this run covers only 50 µm of cut (6 µs), far too short for
    thermal equilibration — heat has not had time to diffuse into the tool bulk; and
    (b) `MFREE_FE_TOOL_CP=20 J/kgK` is 10× smaller than the physical value for WC (~200 J/kgK),
    which reduces the tool’s thermal mass and suppresses long-term heat uptake.
  - The near-perfect closure residual (0.021%) confirms the Phase 2 energy accounting is
    internally consistent and all energy sources are correctly tracked.
  - The zero suppression ratio confirms that at dt=5×10⁻¹⁰ s, the 1°C/step limiter is
    never needed — thermal coupling is well-resolved at this timestep.
  - These numbers form the baseline for any future solver-tightening work
    (calibrated tool material properties, longer runs, bidirectional thermal corrector).

  **All changes are additive or corrective; no solver physics were changed. 5/5 CTest tests pass throughout.**

- 2026-04-20: repo hygiene + workflow docs: establish repeatable commit/PR/CI gates and document them so future work can continue without ambiguity
  - Completed (repo policy / hygiene)
    - Stopped tracking build/results artifacts and ensured they remain ignored (`build/`, `results/**`).
    - Removed `.vscode/settings.json` from version control; added repo-wide example at `.vscode/settings.example.json`.
    - Added `CONTRIBUTING.md` policy clarifying what belongs in Git (IDE config local by default; formatting configs versioned; scripts/docs committed when intentional).
    - Added `.gitattributes` enforcing LF normalization (`* text=auto eol=lf`) to reduce CRLF/LF churn on Windows.
    - Deleted an accidentally created file containing pager/help output and added an ignore pattern to prevent recurrence.
  - Completed (formatting + automation gates)
    - Added `.clang-format` and `.editorconfig` as versioned formatting policy.
    - Added GitHub Actions workflow `.github/workflows/quality.yml` enforcing:
      - basic EditorConfig-style checks (no CR, final newline, no trailing whitespace for tracked files)
      - `clang-format` check against tracked C/C++ sources
    - Added local/CI scripts:
      - `scripts/check_editorconfig_basic.py` (reads files from git index; skips bundled gmsh SDK)
      - `scripts/check_clang_format.py` (reads files from git index; fails cleanly when clang-format not on PATH)
    - Normalized trailing whitespace / final newlines in tracked text assets so the basic EditorConfig gate is green.
  - Completed (developer workflow documentation)
    - Added and expanded `docs/development_workflow.md` with:
      - branching strategy (branch types + naming)
      - code review checklist (author + reviewer) with GitHub UI click-path
      - local build/test gates (CMake + CTest commands)
      - CI/CD gates overview (GitHub Actions quality workflow and local reproduction)
      - release tagging conventions (release tags vs milestone/baseline tags)
      - embedded table-of-contents and a “how to update this doc” roadmap (requirements → drafting → VC workflow → validation → publication)
  - Completed (CI incident response / debugging workflow)
    - Identified failing GitHub Actions runs (quality workflow) for commits 277838a / 78a9dbc / c6f6ed6.
    - Root cause: CI was enforcing a repo-wide `clang-format` sweep, which failed due to formatting drift and vendored third-party sources (notably `Meshing/gmsh-*/**`).
    - Implemented fix: update CI to compute a changed-file list and run formatting gates only on changed files; updated local scripts to accept `--file-list`.
    - Established a repeatable incident protocol:
      - authenticate to GitHub and download workflow logs via GitHub CLI
      - extract per-run / per-file failure mappings into structured artifacts
    - Delivered structured CI failure reports:
      - `docs/ci_failure_report_runs_1_3.md` (summary counts and run links)
      - `docs/ci_failure_report_runs_1_3.csv` (full run → step → file → timestamp → error mapping)
      - `scripts/extract_ci_failures.py` (log parser; handles UTF-16 log encoding)
  - Completed (solver/analysis visibility improvements)
    - Added env parsing to accept tag lists for FE tool boundary fix constraints (comma/semicolon/whitespace-separated); optional anchoring of a single UX node on an anchor physical tag to prevent rigid-mode singularity.
    - Added VTK scalar outputs for FE tool nodes: `fixed_ux` and `fixed_uy` to make boundary conditions visible in ParaView.
  - Validation performed
    - `cmake --build build --config Release` succeeded.
    - `ctest -C Release --test-dir build --output-on-failure` passed (5/5 tests).
    - `python scripts/check_editorconfig_basic.py` passed.
    - Link-target verification for `docs/development_workflow.md` passed (relative links).
  - Current state
    - Working tree clean.
    - Branch `1-thermal-mechanical-solver-fea-tool-and-sph-workpiece-should-be-coupled` pushed with linear commit history; latest includes:
      - CI gating fix (changed-files formatting checks)
      - CI incident playbook and lessons learned in `docs/development_workflow.md`
      - CI failure reports and extraction tooling
  - Ongoing work
    - Open a PR into `main` for the branch and ensure GitHub Actions checks run and pass on the PR.
    - Confirm (or configure) branch protection on `main` to require:
      - at least 2 approvals
      - required status checks (quality workflow) before merge
  - Next steps (action items)
    - Create PR and request two reviewers; include reproduction commands and note which commits are policy/docs vs solver changes.
    - After merge, create an annotated release tag on `main` using the documented convention (`release-<YYYYMMDD>-v<N>`).
    - Announce the workflow changes in team chat with a short summary of new gates and the “how to run locally” commands.
    - Schedule a 30-day follow-up review (issue or calendar) to adjust the workflow doc based on real friction points (clang-format availability on Windows, markdownlint adoption, required checks coverage).
    - Decide whether to keep `Meshing/gmsh-*/**` in formatting scope long-term:
      - Option A: treat as vendored and exclude from formatting enforcement
      - Option B: run a one-time “format-the-world” PR and then enable full-repo enforcement
    - Tighten `.gitignore` policy for generated artifacts and add a pre-commit guard to block accidental commits of excluded paths; validate using `git check-ignore -v` and a staged-file guard test.

- 2026-04-21: docs/session summary + repo hygiene: add a reusable session summary template and ensure Trae IDE metadata stays local
  - Completed
    - Added `docs/session_summary_template.md` to standardize capturing objectives, decisions, action items, and validation evidence.
    - Extended the template with a repository snapshot section (status/untracked/ignored counts), version-control updates, and incident/debug artifacts.
    - Updated `.gitignore` to exclude `.trae/` IDE metadata from version control.
  - Validation performed
    - `python scripts/check_editorconfig_basic.py` passed.
    - `git status --porcelain=v1 --branch` clean after staging/commit.

- 2026-05-30: Fork strategy — PR #4 closed; development stays in the fork

  PR #4 ("Merge Windows-first development line with FE-tool coupling, validation enhancements, and stabilization fixes") was an open pull request targeting `iwf-inspire/mfree_iwf-ul-cut-refine:master` from `GS76/mfree_iwf_ul_cut_refine_windows_ver:master`. It contained 186 commits spanning FE-tool thermo-mechanical coupling, Windows/MSVC compatibility, JSON-based config, gmsh 4.x support, Model 3 refinement controls, energy accounting, and stabilization fixes.

  **Decision:** The PR was closed without merging. Reasoning:
  - The Windows-first development line (`mfree_iwf_ul_cut_refine_windows_ver`) has diverged substantially from the upstream (`mfree_iwf-ul-cut-refine`) — build system (CMake modularization, FetchContent GLM), platform target (Windows/MSVC vs Linux/GCC), and feature additions (JSON config, FE tool coupling, energy accounting) represent a distinct development direction.
  - Upstream sync is not currently needed or requested.

  **Current strategy:**
  - `GS76/mfree_iwf_ul_cut_refine_windows_ver` is the canonical development repository.
  - `origin` (GS76) points to this fork; `upstream` (iwf-inspire) is retained for reference and future pull.
  - If upstream changes are ever needed, pull them selectively via `git fetch upstream; git merge upstream/master` rather than reopening a monolithic merge-back PR.
  - This work log entry serves as the record of this decision.

  - Actions taken
    - PR #4 closed via `gh pr close 4 --repo iwf-inspire/mfree_iwf-ul-cut-refine`.
    - This work log entry created.

  - Validation performed
    - `git --no-pager remote -v` confirms `origin` → GS76 fork, `upstream` → iwf-inspire original.

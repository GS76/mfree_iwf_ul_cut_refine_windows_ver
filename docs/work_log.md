# Work Log

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

  **Full-run baseline measurement (to be recorded after next production run):**

  Run command:
  ```
  $env:MFREE_RESULTS_DIR = "results/baseline_phase5"
  $env:MFREE_LOG_ENERGY  = "1"
  $env:MFREE_THERMAL_T_REF = "298.15"
  .\build\Release\mfree_iwf.exe  # model 1, FE tool, explicit coupled, 100000 steps
  ```

  Then read the final row of `results/baseline_phase5/cutting_energy.csv` and record:

  | Metric | Column in `_energy.csv` | Measured value |
  |---|---|---|
  | Limiter suppression (cumulative) | `cum_suppression_ratio` | *(fill after run)* |
  | Tool-source residual (cumulative) | `cum_tool_source_residual` | *(fill after run)* |
  | Full-system closure residual % | `closure_residual_pct` | *(fill after run)* |
  | Plastic dissipation fraction | `cum_plastic_dissipation / (cum_plastic_dissipation + cum_contact_E_fric_scaled)` | *(fill after run)* |
  | Tool energy fraction | `delta_tool_internal_E / (cum_plastic_dissipation + cum_contact_E_fric_scaled)` | *(fill after run)* |

  These numbers form the baseline for any future solver-tightening work (e.g., switching to a bidirectional thermal corrector or improving `A_eff`).

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

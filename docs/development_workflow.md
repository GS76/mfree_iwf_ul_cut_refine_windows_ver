# Development Workflow (Git + Reproducible Runs)

## Canonical Working Directory

- Always run from the repository root.
- Always invoke the executable via `.\build\Release\mfree_iwf.exe`.
- Do not create or use secondary build folders (`build2`, `cmake-build-*`, etc.).

## One-Time Repo Hygiene (Stop Tracking Build/Results Artifacts)

If `git status` shows `build/` or `results/` files as modified/untracked, clean the index once:

```powershell
git rm -r --cached build results
git add .gitignore
git commit -m "repo: ignore build/results artifacts"
```

## Mandatory Milestone Checkpoint Protocol

Run this after any completed milestone (examples: FE BC validation verified, ParaView batch scripts verified, regression tests green).

1) Enumerate changes

```powershell
git status
```

2) Review changes line-by-line

```powershell
git diff
```

3) Commit with a descriptive message

Option A (simple, all tracked changes):

```powershell
git commit -am "<milestone>: <what> <why>"
```

If you want a hard guard that blocks committing when there are no staged or tracked changes, use:

```powershell
.\scripts\git_commit_am_guard.ps1 -Message "<milestone>: <what> <why>"
```

Option B (granular):

```powershell
git add -p
git commit -m "<milestone>: <what> <why>"
```

4) Tag the baseline so it is always recoverable

```powershell
git tag <milestone>-<YYYYMMDD>-<vN>
```

Example:

```powershell
git tag fe-bc-validate-20260420-v1
```

## Pre-Edit Checklist (Context Drift Prevention)

Before modifying code:

- Re-open and skim:
  - `src/fe_tool.cpp`
  - `src/benchmarks/test_cuttings.cpp`
  - `src/refine_cut_main.cpp`
- Append one bullet to `docs/work_log.md` describing the intent (file + purpose).

## Versioned Results Convention (No Overwrites)

Every run writes into a timestamped folder:

`results/baseline/<YYYYMMDD-HHMM>/<label>/`

Store together:

- env/config used for the run
- stdout/stderr logs
- CSV logs
- VTK outputs
- generated PNGs/plots

Never reuse or overwrite an existing baseline directory.

## Daily Checkpoint Routine

```powershell
git fetch --prune
git status --short --branch
git diff --stat
```

- Commit or stash within 30 minutes of any green test.
- Push at end-of-day:

```powershell
git push origin <branch>
```

## Formatter / Auto-Format Policy

- Disable format-on-save for C++, PowerShell, and Python to prevent unreviewed rewrites.
- Keep formatting changes in dedicated commits only.
- Store formatter configuration in-repo (e.g., `.editorconfig`, `.clang-format`) and update via explicit milestone commits.

## Git Hooks (Auto-Enforced Guards)

Git does not version `.git/hooks/*`, so this repo stores hook templates under `scripts/githooks/` and provides install scripts.

Install (Windows PowerShell):

```powershell
.\scripts\install_githooks.ps1
```

Install (Linux/macOS / Git Bash):

```sh
sh ./scripts/install_githooks.sh
```

What is enforced:

- `pre-commit`: blocks commits when there are no staged changes (prevents empty commits and untracked-only commits).
- `commit-msg`: requires a subject containing `:` and a minimum length (allows `Merge ...` and `Revert ...`).

Optional tests in hook:

- Set `MFREE_PRECOMMIT_RUN_TESTS=1` to run `ctest -C Release --output-on-failure` inside `pre-commit`.

Hook tests (creates a temp repo and confirms commits are blocked/allowed appropriately):

```powershell
python .\scripts\test_githooks.py
```

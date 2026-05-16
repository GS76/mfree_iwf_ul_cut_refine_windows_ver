# Development Workflow (Git + Reproducible Runs)

This document defines a repeatable workflow for developing, reviewing, and releasing changes in this repository, while keeping runs reproducible and diffs reviewable.

## Table of Contents

- [Canonical Working Directory](#canonical-working-directory)
- [Branching Strategy](#branching-strategy)
- [Code Review Checklist](#code-review-checklist)
- [Local Build and Test Gates](#local-build-and-test-gates)
- [Docker (CI-Parity Container)](#docker-ci-parity-container)
- [CI/CD Gates](#cicd-gates)
  - [CI Incident Playbook (Quality Workflow)](#ci-incident-playbook-quality-workflow)
- [One-Time Repo Hygiene](#one-time-repo-hygiene)
  - [Generated Artifacts Policy (.gitignore)](#generated-artifacts-policy-gitignore)
- [Mandatory Milestone Checkpoint Protocol](#mandatory-milestone-checkpoint-protocol)
- [Release Tagging Conventions](#release-tagging-conventions)
- [Versioned Results Convention](#versioned-results-convention)
- [Formatter / Auto-Format Policy](#formatter--auto-format-policy)
- [Git Hooks](#git-hooks)
- [Updating This Document](#updating-this-document)
  - [Lessons Learned](#lessons-learned)

## Canonical Working Directory

- Always run from the repository root.
- Always invoke the executable via `.\build\Release\mfree_iwf.exe`.
- Do not create or use secondary build folders (`build2`, `cmake-build-*`, etc.).

## Branching Strategy

### Branch Types

- `main`: release-ready, protected.
- `feature/*`: normal development branches.
- `fix/*`: bug-fix branches (production-impacting or correctness fixes).
- `docs/*`: documentation-only branches.
- `chore/*`: maintenance, formatting-only, automation-only changes.

### Branch Naming Rules

- Use lowercase and hyphens.
- Include a short scope: `feature/fe-tool-thermal-map`, `fix/contact-penetration-clamp`, `docs/dev-workflow-pr`.

### Create a Feature Branch

```powershell
git fetch --prune
git switch -c docs/development-workflow-update
```

## Code Review Checklist

### Before Opening a PR (Author Checklist)

- Verify `git status` is clean except for intended changes.
- Review the diff locally:

```powershell
git diff
git diff --stat
```

- Run local tests relevant to the change (see [Local Build and Test Gates](#local-build-and-test-gates)).
- Split changes into atomic commits (docs/tooling vs solver changes).
- Ensure no local-only files are being committed (example: `.vscode/settings.json` must remain local; see `CONTRIBUTING.md`).

### PR Requirements (Reviewer Checklist)

- Scope is clear and small enough to review.
- Commit messages are descriptive and scoped.
- Formatting-only changes are isolated to their own commit(s).
- Tests are added or updated where appropriate.
- Reproducibility: the PR description includes exact commands to reproduce or validate.

### GitHub UI Procedure (Open and Review a PR)

1) Push your branch:

```powershell
git push -u origin HEAD
```

2) In GitHub:
   - Navigate to the repository page.
   - Click the **Pull requests** tab.
   - Click **New pull request**.
   - Set base to `main` and compare to your branch.
   - Click **Create pull request**.
3) Require approvals:
   - At least two reviewers approve before merge.
   - Use **Request reviewers** in the PR sidebar.

Branch protection (recommended, one-time):
1) GitHub → **Settings** → **Branches**
2) Add a branch protection rule for `main`
3) Enable:
   - **Require a pull request before merging**
   - **Require approvals** (set to 2)
   - **Dismiss stale approvals when new commits are pushed**
   - **Require status checks to pass before merging**

## Local Build and Test Gates

### Configure (One Time Per Machine or After Major Changes)

```powershell
cmake -S . -B build
```

### Build (Release)

```powershell
cmake --build build --config Release
```

### Run Tests (Release)

```powershell
ctest -C Release --test-dir build --output-on-failure
```

## Docker (CI-Parity Container)

This repository now includes a minimal Linux container setup for reproducible build/test checks aligned with `.github/workflows/quality.yml`.

Scope of this container setup:
- In scope: configure/build/CTest and optional formatting checks.
- Out of scope (initial phase): replacing native Windows long production simulations and host ParaView/pvpython workflows.

### Build the Docker Image (PowerShell)

```powershell
docker build -t mfree-iwf-ci -f Dockerfile .
```

### Run Configure + Build + CTest in Container (PowerShell)

```powershell
docker run --rm -it -v "${PWD}:/workspace" -w /workspace mfree-iwf-ci /bin/bash -lc "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release && ctest --test-dir build -C Release --output-on-failure"
```

If you hit FE mesh load errors in Linux container tests on Windows checkouts (for example `Failed to load MFREE_FE_TOOL_MSH`), run strict CI-parity from an LF-normalized `git archive` snapshot:

```powershell
docker run --rm -it -v "${PWD}:/workspace" mfree-iwf-ci /bin/bash -lc "rm -rf /tmp/src /tmp/mfree-build && mkdir -p /tmp/src && git -C /workspace archive --format=tar HEAD | tar -xf - -C /tmp/src && cmake -S /tmp/src -B /tmp/mfree-build -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/mfree-build --config Release && ctest --test-dir /tmp/mfree-build -C Release --output-on-failure"
```

Equivalent `docker compose` command:

```powershell
docker compose run --rm ci-parity
```

### Run Formatting Gates in Container (PowerShell)

```powershell
docker run --rm -it -v "${PWD}:/workspace" -w /workspace mfree-iwf-ci /bin/bash -lc "python3 scripts/check_editorconfig_basic.py && python3 scripts/check_clang_format.py"
```

### Preprocess-Only Smoke Run with Mounted Outputs (PowerShell)

```powershell
docker run --rm -it -v "${PWD}:/workspace" -w /workspace -e MFREE_PREPROCESS_ONLY=1 -e MFREE_CLEAN_RESULTS=1 -e MFREE_RESULTS_DIR=/workspace/results/docker_preprocess -e MFREE_FE_TOOL_MSH=/workspace/snapshots/tool_plane_strain/meshes/tool_h_0.01mm.msh mfree-iwf-ci /bin/bash -lc "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release && ./build/mfree_iwf -m 1"
```

The results are written to `results/docker_preprocess/` in the host repository via bind mount.

## CI/CD Gates

This repo uses GitHub Actions for automated checks.

### CI Gate

Workflow: `.github/workflows/quality.yml`

- Runs a basic `.editorconfig`-style gate and a `clang-format` gate.
- Also builds the repository and runs CTest to ensure the change set compiles and passes unit tests.
- The formatting gate is enforced on the set of files changed by the push/PR (not the entire repository) to avoid unrelated legacy formatting issues blocking unrelated work.
- If you need to reproduce it locally:

```powershell
python scripts/check_editorconfig_basic.py
python scripts/check_clang_format.py
```

If you want to mirror CI behavior (changed files only), first build a file list (one path per line), then pass it to the scripts:

```powershell
git diff --name-only HEAD~1..HEAD > changed_files.txt
python scripts/check_editorconfig_basic.py --file-list changed_files.txt
python scripts/check_clang_format.py --file-list changed_files.txt
```

### Optional Local Pre-Commit Test Gate

If hooks are installed and you want tests to run automatically during commit:

```powershell
$env:MFREE_PRECOMMIT_RUN_TESTS = "1"
```

### CI Incident Playbook (Quality Workflow)

This section documents a previously observed failure mode and the current recommended debugging procedure.

#### Known Issue: Full-Repo clang-format Failures

Symptom:
- The `quality` workflow fails on commits that do not touch C/C++ formatting (for example, docs-only changes).

Root cause:
- A repo-wide `clang-format` sweep was executed in CI, which caused failures due to existing formatting drift and bundled third-party code (for example, Gmsh SDK sources under `Meshing/gmsh-*/**`).

Implemented solution:
- The CI workflow computes a changed-file list and runs gates against that list only.
- The local check scripts accept `--file-list` so CI and local reproduction use the same file selection.

Evidence and postmortem:
- A per-run, per-file mapping for the original failing runs is stored in:
  - `docs/ci_failure_report_runs_1_3.md`
  - `docs/ci_failure_report_runs_1_3.csv`
- The extraction helper used to generate those reports is `scripts/extract_ci_failures.py`.

#### Debugging Procedure: Extract Failing Files from Actions Logs

Prerequisites:
- GitHub account access to the repository
- GitHub CLI (`gh`) authenticated with `workflow` scope

1) Authenticate:

```powershell
& \"C:\\Program Files\\GitHub CLI\\gh.exe\" auth login --hostname github.com --git-protocol https --web
& \"C:\\Program Files\\GitHub CLI\\gh.exe\" auth status
```

2) List runs for the `quality` workflow:

```powershell
& \"C:\\Program Files\\GitHub CLI\\gh.exe\" run list --repo GS76/mfree_iwf_ul_cut_refine_windows_ver --workflow quality --limit 20
```

3) Download logs for the run IDs you care about:

```powershell
$runIds = @(24688328553,24684121470,24683852166)
foreach ($id in $runIds) {
  & \"C:\\Program Files\\GitHub CLI\\gh.exe\" run view $id --repo GS76/mfree_iwf_ul_cut_refine_windows_ver --log > \"ci_run_$id.log\"
}
```

PowerShell note:
- Do not use placeholders like `<RUN_ID>` in commands. The `<` character is parsed as an operator in PowerShell. Use real numeric IDs or variables as shown above.

4) Generate a structured failure report (CSV + summary Markdown) from the downloaded logs:

```powershell
& \"C:\\Program Files\\GitHub CLI\\gh.exe\" run list --repo GS76/mfree_iwf_ul_cut_refine_windows_ver --workflow quality --limit 10 --json databaseId,displayTitle,headSha,conclusion,createdAt,url,event | Out-File runs_quality.json -Encoding utf8
python scripts/extract_ci_failures.py --runs-json runs_quality.json --log-dir .
```

Implementation note:
- Some `gh run view --log` outputs may be UTF-16 encoded on Windows; the extraction script detects and decodes this automatically.

## One-Time Repo Hygiene

### Stop Tracking Build/Results Artifacts

If `git status` shows `build/` or `results/` files as modified/untracked, clean the index once:

```powershell
git rm -r --cached build results
git add .gitignore
git commit -m "repo: ignore build/results artifacts"
```

### Generated Artifacts Policy (.gitignore)

This repository treats build outputs and generated artifacts as local-only. The exclusion policy is enforced by `.gitignore` and a pre-commit guard.

Excluded (must not be committed):
- `build/`, `build*/`, `cmake-build*/`
- `Debug/`, `Release/`
- `results/**`
- `Meshing/out/`
- generated documentation artifacts (example: `docs/*tree*.txt`)

#### Patch Workflow (Creating or Updating .gitignore Rules)

Prerequisites:
- Working tree clean except for the intended `.gitignore` changes
- Git hooks installed (recommended): `.\scripts\install_githooks.ps1`

1) Create a branch:

```powershell
git fetch --prune
git switch -c chore/gitignore-policy
```

2) Edit `.gitignore` to add or adjust patterns.

3) If any excluded files are currently tracked, untrack them once:

```powershell
git rm -r --cached Debug Release Meshing/out
git add .gitignore
git status
```

4) Validate the ignore rules behave as intended:

```powershell
git check-ignore -v Debug/ Meshing/out/ results/
git check-ignore -v docs/workspace_tree_git_ls_files.txt
```

5) Commit atomically:

```powershell
git add .gitignore
git commit -m "repo: tighten ignore rules for generated artifacts"
```

6) Open a PR and require two approvals (see [Code Review Checklist](#code-review-checklist)).

#### Testing Procedure (Verify Exclusions Work)

1) Confirm excluded directories do not appear as untracked changes:

```powershell
git status
```

2) Confirm no excluded paths are tracked:

```powershell
git ls-files Debug Release Meshing/out | Measure-Object -Line
```

3) Confirm the pre-commit guard blocks accidental staging:

```powershell
mkdir -Force Debug | Out-Null
"test" | Out-File Debug\_ignore_guard_test.txt -Encoding ascii
git add Debug/_ignore_guard_test.txt -f
git commit -m "test: should be blocked by pre-commit"
```

If you must bypass intentionally (rare), set:

```powershell
$env:MFREE_ALLOW_EXCLUDED_STAGE = "1"
```

#### Edge Cases and Guidelines

- Vendored third-party content should not be reformatted or swept into CI gates by default. Prefer to exclude it from formatting enforcement, or isolate a one-time formatting PR with explicit review.
- If a file under an excluded directory must be versioned, do not use `git add -f` as a workflow. Instead:
  - move the file into a versioned location, or
  - add a narrow exception pattern to `.gitignore`, and document the rationale in the PR description.

#### Review and Maintenance Schedule

- Review `.gitignore` rules:
  - monthly, and
  - before each release tag is created.
- Any `.gitignore` change must be reviewed via PR with:
  - two approvals,
  - a note in `docs/work_log.md` describing the change, and
  - validation evidence (commands run and their outcomes).
- Requesting changes:
  - open a tracking issue or PR describing:
    - what is being added/removed from ignore scope
    - why the change is needed
    - how to validate it locally

## Mandatory Milestone Checkpoint Protocol

Run this after any completed milestone (examples: FE BC validation verified, ParaView batch scripts verified, regression tests green).

1) Enumerate changes:

```powershell
git status
```

2) Review changes line-by-line:

```powershell
git diff
```

3) Commit with a descriptive message:

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

4) Tag the baseline so it is always recoverable:

```powershell
git tag <milestone>-<YYYYMMDD>-v<N>
git push origin --tags
```

Example:

```powershell
git tag fe-bc-validate-20260420-v1
git push origin --tags
```

## Release Tagging Conventions

Use lightweight, searchable tags that encode the date and a monotonic counter.

### Release Tags

- `release-<YYYYMMDD>-v<N>` for repository releases.
- Create an annotated tag from the merge commit on `main`:

```powershell
git switch main
git pull --ff-only
git tag -a release-20260420-v1 -m "release: 20260420 v1"
git push origin --tags
```

### Milestone/Baseline Tags

- Use milestone tags for experimental checkpoints that are not releases:
  - `fe-bc-validate-<YYYYMMDD>-v<N>`
  - `thermal-coupling-<YYYYMMDD>-v<N>`

## Versioned Results Convention

Every run writes into a timestamped folder:

`results/baseline/<YYYYMMDD-HHMM>/<label>/`

Store together:

- env/config used for the run
- stdout/stderr logs
- CSV logs
- VTK outputs
- generated PNGs/plots

Never reuse or overwrite an existing baseline directory.

## Formatter / Auto-Format Policy

- Disable format-on-save for C++, PowerShell, and Python to prevent unreviewed rewrites.
- Keep formatting changes in dedicated commits only.
- Store formatter configuration in-repo (e.g., `.editorconfig`, `.clang-format`) and update via explicit milestone commits.

## Git Hooks

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

Hook tests (creates a temp repo and confirms commits are blocked/allowed appropriately):

```powershell
python .\scripts\test_githooks.py
```

## Updating This Document

This section defines the roadmap and procedure for changing `docs/development_workflow.md`.

### 1) Requirements Gathering

1) Interview stakeholders:
   - Solver developers (SPH/FE coupling)
   - CI/CD owner (GitHub Actions, build agents)
   - Reviewers/maintainers (merge policy, releases)
2) Audit current automation:
   - Open `.github/workflows/quality.yml` and list every enforced gate.
   - Confirm whether `main` has branch protection and required approvals.
3) Identify outdated/missing content:
   - Compare this document against `CONTRIBUTING.md`, `scripts/githooks/*`, and current workflows.
   - Record gaps as a checklist in your PR description (or a tracking issue if large).

### 2) Content Drafting

Draft updates as actionable procedures:

- Add missing sections (branching strategy, code review checklist, CI gates, release tags).
- For each procedure, include:
  - exact CLI commands, or
  - explicit GitHub UI navigation and button names

### 3) Version-Control Workflow (Docs Update)

1) Create a branch:

```powershell
git fetch --prune
git switch -c docs/development-workflow-update
```

2) Make atomic commits:

```powershell
git add docs/development_workflow.md
git commit -m "docs: update development workflow (branching/review/release)"
```

3) Open a PR and require at least two approvals (see [Code Review Checklist](#code-review-checklist)).

### 4) Validation

Minimum required (repo-native):

```powershell
python scripts/check_editorconfig_basic.py
```

Optional markdown lint (if Node + npm are available):

```powershell
npm install -g markdownlint-cli2
markdownlint-cli2 docs/development_workflow.md
```

Verify every markdown link target exists (relative links only):

```powershell
python -c "import os,re,sys; p='docs/development_workflow.md'; s=open(p,'r',encoding='utf-8').read(); ok=True; \
 import pathlib; base=pathlib.Path(p).parent; \
 for m in re.finditer(r'\\[[^\\]]*\\]\\(([^)]+)\\)', s): \
  u=m.group(1).split('#',1)[0].strip(); \
  if not u or '://' in u or u.startswith('mailto:'): continue; \
  q=(base/u).resolve(); \
  if not q.exists(): print('missing:',u); ok=False; \
 sys.exit(0 if ok else 1)"
```

Dry-run every documented command in a clean environment:

1) Create a fresh clone:

```powershell
cd $env:TEMP
git clone <repo-url> mfree_clean
cd mfree_clean
```

2) Run every CLI block in this document in order and confirm it succeeds.

### 5) Publication

1) Merge to `main` after:
   - CI passes
   - at least two approvals are recorded
2) Tag the release (see [Release Tagging Conventions](#release-tagging-conventions)).
3) Announce in team chat:
   - link to the PR
   - highlight any new required gates or policy changes
4) Schedule a follow-up review in 30 days:
   - add a calendar reminder or create an issue titled: `docs: review development workflow`

### Lessons Learned

- Incremental enforcement beats “format-the-world” gates: repository-wide formatting checks can cause unrelated work to fail due to historical drift or vendored third-party sources.
- CI and local reproduction must align: scripts and workflows should share the same file selection behavior (changed-file list), so failures can be reproduced deterministically.
- Prefer structured incident artifacts: saving a per-run/per-file CSV makes future root-cause analysis faster than re-reading raw logs.
- PowerShell ergonomics matter: avoid placeholder syntax that conflicts with PowerShell parsing rules; prefer variables and arrays.

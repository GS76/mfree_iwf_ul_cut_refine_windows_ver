# Contributing

This repository prioritizes reproducible runs, minimal diffs, and commit history that cleanly separates tooling changes from solver/physics changes.

For day-to-day workflow details, see `docs/development_workflow.md`.

## What Belongs In Git

### Editor/IDE Configuration

- Default: keep personal editor settings local.
- Do not commit `.vscode/settings.json`.
- Use `.vscode/settings.example.json` as the optional, reviewed, repository-wide recommendation.
- If the team elects to commit shared settings, remove the ignore rule and treat any changes as a normal code change requiring PR review.

Personal-only ignores:

- Prefer adding personal patterns to `.git/info/exclude` rather than committing ignore rules that are only relevant to one machine.

### Formatting Rules

- `.clang-format` and `.editorconfig` are repository policy and must be committed.
- Keep formatting-only changes in dedicated commits.

Local enforcement:

- Disable format-on-save to prevent unreviewed bulk rewrites.
- Run formatting checks before committing.

### Documentation and Automation

- Commit `docs/**` and `scripts/**` only when intentional, up-to-date, and runnable.
- Each script should have a clear name and be referenced from `docs/development_workflow.md` or a short README in `scripts/` when needed.

### Source Code Changes

- Treat `src/**` changes as production changes.
- Prefer small, atomic commits with descriptive messages.
- Add or update automated tests where feasible; do not rely only on manual runs.

## Hooks and Guards

This repo stores hook templates in `scripts/githooks/` and provides install scripts:

- Windows PowerShell: `.\scripts\install_githooks.ps1`
- Linux/macOS / Git Bash: `sh ./scripts/install_githooks.sh`

Optional local test gate:

- Set `MFREE_PRECOMMIT_RUN_TESTS=1` to run `ctest -C Release --output-on-failure` during `pre-commit` (requires a configured `./build`).

## Repeatable Change Checklist

- Run `git status` and `git diff` before staging.
- Stage intentionally (prefer `git add -p`).
- Verify formatting checks pass.
- Verify relevant tests pass (or document why tests are not applicable).
- Keep commits scoped: tooling/docs vs solver changes.

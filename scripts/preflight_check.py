import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path


def run(cmd, *, cwd):
    p = subprocess.run(cmd, cwd=cwd)
    return p.returncode


def capture(cmd, *, cwd):
    p = subprocess.run(
        cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    if p.returncode != 0:
        sys.stderr.write(p.stderr)
        raise RuntimeError(f"command failed ({p.returncode}): {' '.join(cmd)}")
    return p.stdout


def git_files(repo, all_files):
    if all_files:
        out = capture(["git", "ls-files"], cwd=repo)
        return sorted({p for p in out.splitlines() if p})

    files = set()
    commands = [
        ["git", "diff", "--name-only", "--diff-filter=ACMR", "HEAD"],
        ["git", "ls-files", "--others", "--exclude-standard"],
    ]
    for cmd in commands:
        try:
            out = capture(cmd, cwd=repo)
        except RuntimeError:
            continue
        files.update(p for p in out.splitlines() if p)
    return sorted(files)


def write_file_list(repo, files):
    fd, path = tempfile.mkstemp(prefix="mfree_preflight_", suffix=".txt", dir=repo)
    with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as f:
        for rel in files:
            f.write(rel + "\n")
    return path


def main():
    parser = argparse.ArgumentParser(
        description="Run local quality checks before staging or committing."
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Check all tracked files instead of changed/untracked files",
    )
    parser.add_argument(
        "--build",
        action="store_true",
        help="Also configure and build the Release target",
    )
    parser.add_argument(
        "--test", action="store_true", help="Also run CTest after configuring/building"
    )
    parser.add_argument(
        "--skip-format",
        action="store_true",
        help="Skip clang-format and basic EditorConfig checks",
    )
    args = parser.parse_args()

    repo = Path(__file__).resolve().parents[1]
    py = sys.executable

    failures = []

    if not args.skip_format:
        files = git_files(repo, args.all)
        if files:
            file_list = write_file_list(repo, files)
            try:
                checks = [
                    [
                        py,
                        "scripts/check_editorconfig_basic.py",
                        "--worktree",
                        "--file-list",
                        file_list,
                    ],
                    [
                        py,
                        "scripts/check_clang_format.py",
                        "--worktree",
                        "--file-list",
                        file_list,
                    ],
                ]
                for cmd in checks:
                    rc = run(cmd, cwd=repo)
                    if rc != 0:
                        failures.append(" ".join(cmd))
            finally:
                try:
                    os.remove(file_list)
                except OSError:
                    pass
        else:
            print("preflight: no changed or untracked files to check")

    if args.build or args.test:
        configure = [
            "cmake",
            "-S",
            ".",
            "-B",
            "build",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            "-DMFREE_WARNINGS_AS_ERRORS=ON",
        ]
        build = ["cmake", "--build", "build", "--config", "Release"]
        for cmd in [configure, build]:
            rc = run(cmd, cwd=repo)
            if rc != 0:
                failures.append(" ".join(cmd))
                break

    if args.test:
        cmd = ["ctest", "-C", "Release", "--test-dir", "build", "--output-on-failure"]
        rc = run(cmd, cwd=repo)
        if rc != 0:
            failures.append(" ".join(cmd))

    if failures:
        sys.stderr.write("preflight: failed checks:\n")
        for failure in failures:
            sys.stderr.write(f"  {failure}\n")
        return 1

    print("preflight: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

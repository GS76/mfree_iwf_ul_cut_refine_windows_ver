import argparse
import os
import subprocess
import sys


def run(cmd, *, cwd):
    p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    return p.returncode, p.stdout, p.stderr


def run_bytes(cmd, *, cwd):
    p = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.returncode, p.stdout, p.stderr


def looks_text(data):
    return b"\x00" not in data


def should_check(path):
    if path.startswith("Meshing/gmsh-") or path.startswith("Meshing\\gmsh-"):
        return False
    base = os.path.basename(path)
    if base in {"CMakeLists.txt", ".clang-format", ".editorconfig"}:
        return True
    _, ext = os.path.splitext(base.lower())
    return ext in {
        ".c",
        ".cc",
        ".cpp",
        ".cxx",
        ".h",
        ".hh",
        ".hpp",
        ".hxx",
        ".inl",
        ".cmake",
        ".md",
        ".json",
        ".py",
        ".ps1",
        ".sh",
        ".txt",
        ".yml",
        ".yaml",
    }


def read_file_list(path):
    with open(path, "r", encoding="utf-8") as f:
        out = []
        for line in f:
            s = line.strip()
            if s:
                out.append(s)
        return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--file-list", help="Newline-separated paths (repo-relative) to check"
    )
    ap.add_argument(
        "--worktree",
        action="store_true",
        help="Check working-tree file contents instead of staged contents",
    )
    args = ap.parse_args()

    repo = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

    if args.file_list:
        files = [p for p in read_file_list(args.file_list) if should_check(p)]
    else:
        rc, out, err = run(["git", "ls-files"], cwd=repo)
        if rc != 0:
            sys.stderr.write(err)
            return rc
        files = [p for p in out.splitlines() if should_check(p)]
    if not files:
        return 0

    errors = []
    for rel in files:
        if args.worktree:
            path = os.path.join(repo, rel)
            if not os.path.isfile(path):
                continue
            with open(path, "rb") as f:
                data = f.read()
        else:
            rc, data, err = run_bytes(["git", "show", f":{rel}"], cwd=repo)
            if rc != 0:
                continue

        if not looks_text(data):
            continue

        if b"\r" in data:
            errors.append((rel, "contains CR (expected LF)"))

        if len(data) > 0 and not data.endswith(b"\n"):
            errors.append((rel, "missing final newline"))

        lines = data.splitlines(keepends=True)
        for i, line in enumerate(lines, start=1):
            if line.endswith(b"\n"):
                core = line[:-1]
                if core.endswith(b"\r"):
                    core = core[:-1]
                if len(core) > 0 and core[-1:] in (b" ", b"\t"):
                    errors.append((rel, f"trailing whitespace at line {i}"))

    if errors:
        sys.stderr.write("basic .editorconfig check failed:\n")
        for path, msg in errors:
            sys.stderr.write(f"  {path}: {msg}\n")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

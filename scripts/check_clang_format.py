import os
import subprocess
import sys


def run(cmd, *, cwd):
    p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    return p.returncode, p.stdout, p.stderr


def run_bytes(cmd, *, cwd):
    p = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.returncode, p.stdout, p.stderr


def is_cpp_file(path):
    _, ext = os.path.splitext(path.lower())
    return ext in {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl"}


def main():
    repo = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

    rc, out, err = run(["git", "ls-files"], cwd=repo)
    if rc != 0:
        sys.stderr.write(err)
        return rc

    files = [p for p in out.splitlines() if is_cpp_file(p)]
    if not files:
        return 0

    try:
        rc, _, err = run(["clang-format", "--version"], cwd=repo)
    except FileNotFoundError:
        sys.stderr.write("clang-format is required on PATH.\n")
        return 2
    if rc != 0:
        sys.stderr.write("clang-format is required on PATH.\n")
        sys.stderr.write(err)
        return 2

    bad = []
    for rel in files:
        rc, src, err = run_bytes(["git", "show", f":{rel}"], cwd=repo)
        if rc != 0:
            sys.stderr.write(f"{rel}: failed to read from git index\n")
            sys.stderr.write(err.decode("utf-8", errors="replace"))
            return 2

        p = subprocess.run(
            ["clang-format", "--style=file", f"--assume-filename={rel}"],
            cwd=repo,
            input=src,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        if p.returncode != 0:
            sys.stderr.write(f"{rel}: clang-format failed\n")
            sys.stderr.write(p.stderr.decode("utf-8", errors="replace"))
            return 2

        if p.stdout != src:
            bad.append(rel)

    if bad:
        sys.stderr.write("clang-format check failed for:\n")
        for p in bad:
            sys.stderr.write(f"  {p}\n")
        sys.stderr.write("\nFix by running clang-format using the repo .clang-format.\n")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

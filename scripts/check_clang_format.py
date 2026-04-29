import argparse
import os
import shutil
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


def should_check_file(path):
    """Return True if the file should be checked, False if it's excluded (e.g., vendor code)."""
    # Exclude third-party libraries and vendor directories
    excluded_prefixes = (
        "Meshing/gmsh-",  # Exclude Gmsh SDK and other bundled tools
    )
    return not any(path.startswith(prefix) for prefix in excluded_prefixes)


def read_file_list(path):
    with open(path, "r", encoding="utf-8") as f:
        out = []
        for line in f:
            s = line.strip()
            if s:
                out.append(s)
        return out


def add_clang_format_search_paths(repo):
    """Add common clang-format install locations to PATH for this process only."""
    env = os.environ
    candidates = [
        os.path.join(repo, "tools"),
        os.path.join(repo, "tools", "LLVM", "bin"),
        os.path.join(repo, "tools", "llvm", "bin"),
    ]

    for key in ("ProgramFiles", "ProgramFiles(x86)", "LOCALAPPDATA"):
        base = env.get(key)
        if base:
            candidates.append(os.path.join(base, "LLVM", "bin"))
            candidates.append(os.path.join(base, "Programs", "LLVM", "bin"))

    user_profile = env.get("USERPROFILE")
    if user_profile:
        candidates.extend(
            [
                os.path.join(user_profile, "scoop", "shims"),
                os.path.join(user_profile, "scoop", "apps", "llvm", "current", "bin"),
            ]
        )

    candidates.extend(
        [
            r"C:\Program Files\LLVM\bin",
            r"C:\Program Files (x86)\LLVM\bin",
            r"C:\ProgramData\chocolatey\bin",
            r"C:\msys64\clang64\bin",
            r"C:\msys64\mingw64\bin",
            r"C:\msys64\ucrt64\bin",
        ]
    )

    existing = []
    seen = set()
    for path in env.get("PATH", "").split(os.pathsep):
        normalized = os.path.normcase(os.path.abspath(path)) if path else path
        if normalized not in seen:
            seen.add(normalized)
            existing.append(path)

    for path in candidates:
        if os.path.isdir(path):
            normalized = os.path.normcase(os.path.abspath(path))
            if normalized not in seen:
                seen.add(normalized)
                existing.append(path)

    env["PATH"] = os.pathsep.join(existing)


def find_clang_format(repo):
    override = os.environ.get("CLANG_FORMAT") or os.environ.get("MFREE_CLANG_FORMAT")
    if override:
        return override

    add_clang_format_search_paths(repo)
    return shutil.which("clang-format") or shutil.which("clang-format.exe")


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
        files = [
            p
            for p in read_file_list(args.file_list)
            if is_cpp_file(p) and should_check_file(p)
        ]
    else:
        rc, out, err = run(["git", "ls-files"], cwd=repo)
        if rc != 0:
            sys.stderr.write(err)
            return rc
        files = [p for p in out.splitlines() if is_cpp_file(p) and should_check_file(p)]
    if not files:
        return 0

    clang_format = find_clang_format(repo)
    if not clang_format:
        sys.stderr.write(
            "clang-format is required on PATH or in a common Windows LLVM install location.\n"
        )
        sys.stderr.write(
            "You can also set CLANG_FORMAT or MFREE_CLANG_FORMAT to the full clang-format executable path.\n"
        )
        return 2

    try:
        rc, _, err = run([clang_format, "--version"], cwd=repo)
    except FileNotFoundError:
        sys.stderr.write(f"clang-format was not found: {clang_format}\n")
        return 2
    if rc != 0:
        sys.stderr.write(f"clang-format failed to run: {clang_format}\n")
        sys.stderr.write(err)
        return 2

    bad = []
    for rel in files:
        if args.worktree:
            path = os.path.join(repo, rel)
            if not os.path.isfile(path):
                continue
            with open(path, "rb") as f:
                src = f.read()
        else:
            rc, src, err = run_bytes(["git", "show", f":{rel}"], cwd=repo)
            if rc != 0:
                continue

        p = subprocess.run(
            [clang_format, "--style=file", f"--assume-filename={rel}"],
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
        sys.stderr.write(
            "\nFix by running clang-format using the repo .clang-format.\n"
        )
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

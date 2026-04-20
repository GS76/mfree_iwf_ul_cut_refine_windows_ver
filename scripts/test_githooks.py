import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def run(cmd, cwd, check=True):
	p = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, shell=False)
	if check and p.returncode != 0:
		raise RuntimeError(f"command failed ({p.returncode}): {' '.join(cmd)}\n{p.stdout}")
	return p.returncode, p.stdout


def main() -> int:
	repo_root = Path(__file__).resolve().parents[1]
	install_ps1 = repo_root / "scripts" / "install_githooks.ps1"
	install_sh = repo_root / "scripts" / "install_githooks.sh"

	tmp = Path(tempfile.mkdtemp(prefix="githooks_test_"))
	try:
		run(["git", "init"], cwd=tmp)
		run(["git", "config", "user.email", "test@example.com"], cwd=tmp)
		run(["git", "config", "user.name", "Test"], cwd=tmp)

		(tmp / "a.txt").write_text("init\n", encoding="utf-8")
		run(["git", "add", "a.txt"], cwd=tmp)
		run(["git", "commit", "-m", "init: baseline"], cwd=tmp)

		scripts_dir = tmp / "scripts"
		scripts_dir.mkdir(parents=True, exist_ok=True)
		for p in [install_ps1, install_sh]:
			shutil.copy2(p, scripts_dir / p.name)
		shutil.copytree(repo_root / "scripts" / "githooks", scripts_dir / "githooks")

		if os.name == "nt":
			run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(scripts_dir / "install_githooks.ps1")], cwd=tmp)
		else:
			run(["sh", str(scripts_dir / "install_githooks.sh")], cwd=tmp)

		(tmp / "b.txt").write_text("untracked only\n", encoding="utf-8")
		rc, out = run(["git", "commit", "-m", "test: should block"], cwd=tmp, check=False)
		if rc == 0:
			raise RuntimeError("expected commit to be blocked when no staged changes exist")

		run(["git", "add", "b.txt"], cwd=tmp)
		rc, out = run(["git", "commit", "-m", "badmessage"], cwd=tmp, check=False)
		if rc == 0:
			raise RuntimeError("expected commit to be blocked by commit-msg rule (missing ':')")

		rc, out = run(["git", "commit", "-m", "hook: allow staged change"], cwd=tmp, check=False)
		if rc != 0:
			raise RuntimeError(f"expected commit to succeed with valid message and staged changes\n{out}")

		return 0
	finally:
		shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
	sys.exit(main())


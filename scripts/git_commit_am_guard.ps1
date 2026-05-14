param(
	[string]$Message = "",
	[switch]$AllowUntrackedOnly = $false,
	[switch]$SelfTest = $false
)

$ErrorActionPreference = "Stop"

function Require-RepoRoot {
	if (-not (Test-Path ".git")) { throw "run from repo root (missing .git)" }
}

function Has-StagedChanges {
	git diff --cached --quiet
	return ($LASTEXITCODE -ne 0)
}

function Has-UnstagedTrackedChanges {
	git diff --quiet
	return ($LASTEXITCODE -ne 0)
}

function Has-UntrackedFiles {
	$u = git status --porcelain | Where-Object { $_ -match '^\?\?' }
	return ($null -ne $u -and $u.Count -gt 0)
}

function Guard-And-Commit {
	param([string]$CommitMessage)

	$hasStaged = Has-StagedChanges
	$hasUnstaged = Has-UnstagedTrackedChanges
	$hasUntracked = Has-UntrackedFiles

	if (-not $hasStaged -and -not $hasUnstaged) {
		if ($hasUntracked -and -not $AllowUntrackedOnly) {
			throw "blocked: only untracked files exist. 'git commit -am' will not include them. Use 'git add -A' or 'git add -p' first."
		}
		throw "blocked: working tree is clean (no staged changes and no tracked modifications)."
	}

	if (-not $CommitMessage -or $CommitMessage.Trim().Length -eq 0) {
		throw 'blocked: commit message missing. Use -Message "<milestone>: <what> <why>".'
	}

	if ($hasUnstaged) {
		git commit -am $CommitMessage
		return
	}

	git commit -m $CommitMessage
}

function Self-Test {
	$base = Join-Path $env:TEMP ("git_commit_guard_test_" + [guid]::NewGuid().ToString("N"))
	New-Item -ItemType Directory -Force -Path $base | Out-Null
	Push-Location $base
	try {
		git init | Out-Null
		"init" | Set-Content -Encoding UTF8 -Path "a.txt"
		git add a.txt
		git commit -m "init" | Out-Null

		$ok = 0
		$fail = 0

		try { Guard-And-Commit -CommitMessage "clean" ; $fail++ } catch { $ok++ }

		"change" | Add-Content -Encoding UTF8 -Path "a.txt"
		try { Guard-And-Commit -CommitMessage "unstaged" ; $ok++ } catch { $fail++ }

		"stage" | Add-Content -Encoding UTF8 -Path "a.txt"
		git add a.txt
		try { Guard-And-Commit -CommitMessage "staged" ; $ok++ } catch { $fail++ }

		"new" | Set-Content -Encoding UTF8 -Path "b.txt"
		try { Guard-And-Commit -CommitMessage "untracked" ; $fail++ } catch { $ok++ }

		if ($fail -ne 0) { throw "self-test failed ($fail failing cases, $ok passing cases)" }
	} finally {
		Pop-Location
		Remove-Item -Recurse -Force $base
	}
}

Require-RepoRoot

$env:GIT_PAGER = "cat"
$env:PAGER = "cat"

if ($SelfTest) {
	Self-Test
	Write-Host "self-test: OK"
	exit 0
}

Guard-And-Commit -CommitMessage $Message

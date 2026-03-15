param(
  [int]$Model = 1,
  [double]$CooldownHconv = 25.0,
  [int]$ValidationTimeoutSeconds = 300,
  [int]$FullRunTimeoutSeconds = 0,
  [int]$RunFull = 1,
  [int]$SkipBuild = 0,
  [int]$SkipCTest = 0
)

$ErrorActionPreference = "Stop"

function Invoke-ProcessWithTimeout {
  param(
    [string]$FilePath,
    [string[]]$ArgumentList,
    [string]$StdoutPath,
    [string]$StderrPath,
    [int]$TimeoutSeconds
  )

  $stdoutDir = Split-Path -Parent $StdoutPath
  if ($stdoutDir -and -not (Test-Path $stdoutDir)) { New-Item -ItemType Directory -Path $stdoutDir | Out-Null }
  $stderrDir = Split-Path -Parent $StderrPath
  if ($stderrDir -and -not (Test-Path $stderrDir)) { New-Item -ItemType Directory -Path $stderrDir | Out-Null }

  $p = Start-Process -FilePath $FilePath -ArgumentList $ArgumentList -NoNewWindow -PassThru -RedirectStandardOutput $StdoutPath -RedirectStandardError $StderrPath

  if ($TimeoutSeconds -le 0) {
    $p.WaitForExit() | Out-Null
  } else {
    $ok = $p.WaitForExit($TimeoutSeconds * 1000)
    if (-not $ok) {
      try { Stop-Process -Id $p.Id -Force } catch {}
      throw "Timeout after $TimeoutSeconds seconds: $FilePath $($ArgumentList -join ' ')"
    }
  }

  $code = $p.ExitCode
  if ($null -ne $code -and $code -ne 0) {
    throw "Non-zero exit code ${code}: $FilePath $($ArgumentList -join ' ')"
  }
}

function Assert-FileExists {
  param([string]$Path)
  if (-not (Test-Path $Path)) { throw "Missing required file: $Path" }
}

function Assert-AnyMatch {
  param([string]$GlobPattern)
  $matches = Get-ChildItem -Path $GlobPattern -ErrorAction SilentlyContinue
  if (-not $matches -or $matches.Count -lt 1) { throw "Missing required outputs matching: $GlobPattern" }
}

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $repoRoot

$resultsDir = Join-Path $repoRoot "results"
if (-not (Test-Path $resultsDir)) { New-Item -ItemType Directory -Path $resultsDir | Out-Null }

$logDir = Join-Path $resultsDir "pipeline_logs"
if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir | Out-Null }

if ($SkipBuild -eq 0) {
  Invoke-ProcessWithTimeout -FilePath "cmake" -ArgumentList @("--build","build","--target","mfree_iwf","validate_omp","test_property_interpolation","-j","4") -StdoutPath (Join-Path $logDir "build_stdout.txt") -StderrPath (Join-Path $logDir "build_stderr.txt") -TimeoutSeconds 0
}

if ($SkipCTest -eq 0) {
  Invoke-ProcessWithTimeout -FilePath "ctest" -ArgumentList @("--test-dir","build","--output-on-failure") -StdoutPath (Join-Path $logDir "ctest_stdout.txt") -StderrPath (Join-Path $logDir "ctest_stderr.txt") -TimeoutSeconds $ValidationTimeoutSeconds
}

$exe = Join-Path $repoRoot "build\mfree_iwf.exe"
Assert-FileExists $exe

$validationArgs = @("--smoke","--cooldown","--cooldown-hconv",$CooldownHconv,"-m",$Model)
Invoke-ProcessWithTimeout -FilePath $exe -ArgumentList $validationArgs -StdoutPath (Join-Path $logDir "validation_stdout.txt") -StderrPath (Join-Path $logDir "validation_stderr.txt") -TimeoutSeconds $ValidationTimeoutSeconds

Assert-FileExists (Join-Path $resultsDir "cooldown_rate.csv")
Assert-FileExists (Join-Path $resultsDir "cooldown_summary.txt")
Assert-AnyMatch (Join-Path $resultsDir "out_*.vtk")
Assert-AnyMatch (Join-Path $resultsDir "cooldown_*.vtk")
Assert-AnyMatch (Join-Path $resultsDir "tool_*.vtk")
Assert-AnyMatch (Join-Path $resultsDir "cooldown_tool_*.vtk")

$statusPath = Join-Path $resultsDir "pipeline_status.txt"
Set-Content -Path $statusPath -Value "VALIDATION_OK"

if ($RunFull -ne 0) {
  $fullArgs = @("--cooldown","--cooldown-hconv",$CooldownHconv,"-m",$Model)
  $fullStdout = Join-Path $logDir "fullrun_stdout.txt"
  $fullStderr = Join-Path $logDir "fullrun_stderr.txt"
  Invoke-ProcessWithTimeout -FilePath $exe -ArgumentList $fullArgs -StdoutPath $fullStdout -StderrPath $fullStderr -TimeoutSeconds $FullRunTimeoutSeconds
}

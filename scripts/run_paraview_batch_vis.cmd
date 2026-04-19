@echo off
setlocal EnableExtensions

if "%~1"=="" goto :usage

set PVPYTHON=
for %%I in (pvpython.exe) do set PVPYTHON=%%~$PATH:I

if not "%PVPYTHON%"=="" goto :run

if not "%PARAVIEW_HOME%"=="" (
  if exist "%PARAVIEW_HOME%\bin\pvpython.exe" set PVPYTHON=%PARAVIEW_HOME%\bin\pvpython.exe
)

if "%PVPYTHON%"=="" (
  echo pvpython.exe not found. Install ParaView or add pvpython to PATH. You can also set PARAVIEW_HOME to the ParaView install directory.
  exit /b 2
)

:run
set SCRIPT=%~dp0paraview_batch_vis.py
if not exist "%SCRIPT%" (
  echo Missing script: %SCRIPT%
  exit /b 2
)

"%PVPYTHON%" "%SCRIPT%" %*
exit /b %ERRORLEVEL%

:usage
echo Usage:
echo   scripts\run_paraview_batch_vis.cmd --fe-vtk PATH --top-csv PATH --rear-csv PATH --out-dir PATH [options]
exit /b 2


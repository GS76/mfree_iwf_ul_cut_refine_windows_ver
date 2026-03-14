@echo off
setlocal enabledelayedexpansion

set "BUILD_DIR=build"
set "CONFIG=Release"
set "PREFIX="
set "GENERATOR="
set "PARALLEL="
set "CLEAN=0"
set "PACKAGE=0"

:parse
if "%~1"=="" goto done_parse
if "%~1"=="--build-dir" (set "BUILD_DIR=%~2" & shift & shift & goto parse)
if "%~1"=="--config" (set "CONFIG=%~2" & shift & shift & goto parse)
if "%~1"=="--prefix" (set "PREFIX=%~2" & shift & shift & goto parse)
if "%~1"=="--generator" (set "GENERATOR=%~2" & shift & shift & goto parse)
if "%~1"=="--parallel" (set "PARALLEL=%~2" & shift & shift & goto parse)
if "%~1"=="--clean" (set "CLEAN=1" & shift & goto parse)
if "%~1"=="--package" (set "PACKAGE=1" & shift & goto parse)
echo Unknown argument: %~1
exit /b 2

:done_parse

if "%PREFIX%"=="" (
  set "PREFIX=%CD%\install"
)

where cmake >nul 2>nul
if errorlevel 1 (
  echo cmake not found. Install CMake ^>= 3.16 and ensure it is in PATH.
  exit /b 1
)

if "%GENERATOR%"=="" (
  where ninja >nul 2>nul
  if errorlevel 0 (
    set "GENERATOR=Ninja"
  ) else (
    where mingw32-make >nul 2>nul
    if errorlevel 0 (
      set "GENERATOR=MinGW Makefiles"
    ) else (
      where nmake >nul 2>nul
      if errorlevel 0 (
        set "GENERATOR=NMake Makefiles"
      ) else (
        set "GENERATOR=Ninja"
      )
    )
  )
)

if "%GENERATOR%"=="Ninja" (
  where ninja >nul 2>nul
  if errorlevel 1 (
    set "TOOLS_DIR=.tools\ninja"
    set "NINJA_EXE=!TOOLS_DIR!\ninja.exe"
    if not exist "!NINJA_EXE!" (
      mkdir "!TOOLS_DIR!" >nul 2>nul
      echo Ninja not found. Downloading a local copy...
      powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "$u='https://github.com/ninja-build/ninja/releases/latest/download/ninja-win.zip';" ^
        "$d='!TOOLS_DIR!'; $z=(Join-Path $d 'ninja-win.zip');" ^
        "Invoke-WebRequest -Uri $u -OutFile $z; Expand-Archive -Path $z -DestinationPath $d -Force; Remove-Item $z -Force"
      if errorlevel 1 (
        echo Failed to download Ninja. Install Ninja or select another generator.
        exit /b 1
      )
    )
    set "PATH=%CD%\!TOOLS_DIR!;%PATH%"
  )
)

if "%CLEAN%"=="1" (
  if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

set "CONFIG_ARGS="
echo %GENERATOR% | findstr /i "Visual Studio" >nul
if errorlevel 1 (
  if /i not "%GENERATOR%"=="Ninja Multi-Config" (
    set "CONFIG_ARGS=-DCMAKE_BUILD_TYPE=%CONFIG%"
  )
)

set "PREFIX_ARGS="
if not "%PREFIX%"=="" (
  set "PREFIX_ARGS=-DCMAKE_INSTALL_PREFIX=%PREFIX%"
)

cmake -S . -B "%BUILD_DIR%" -G "%GENERATOR%" %CONFIG_ARGS% %PREFIX_ARGS%
if errorlevel 1 exit /b 1

set "BUILD_ARGS="
if not "%PARALLEL%"=="" (
  set "BUILD_ARGS=--parallel %PARALLEL%"
)

cmake --build "%BUILD_DIR%" --config "%CONFIG%" %BUILD_ARGS%
if errorlevel 1 exit /b 1

cmake --install "%BUILD_DIR%" --config "%CONFIG%"
if errorlevel 1 exit /b 1

if "%PACKAGE%"=="1" (
  pushd "%BUILD_DIR%"
  cpack -C "%CONFIG%"
  popd
)

echo Done.

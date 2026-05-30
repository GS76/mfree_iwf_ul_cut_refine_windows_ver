#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
CONFIG="${CONFIG:-Release}"
PREFIX="${PREFIX:-}"
GENERATOR="${GENERATOR:-}"
PARALLEL="${PARALLEL:-}"
CLEAN=0
PACKAGE=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) BUILD_DIR="$2"; shift 2 ;;
    --config) CONFIG="$2"; shift 2 ;;
    --prefix) PREFIX="$2"; shift 2 ;;
    --generator) GENERATOR="$2"; shift 2 ;;
    --parallel) PARALLEL="$2"; shift 2 ;;
    --clean) CLEAN=1; shift ;;
    --package) PACKAGE=1; shift ;;
    *) echo "Unknown argument: $1" >&2; exit 2 ;;
  esac
done

if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake not found. Install CMake >= 3.16 and ensure it is in PATH." >&2
  exit 1
fi

if [[ -z "${GENERATOR}" ]]; then
  if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
  elif command -v make >/dev/null 2>&1; then
    GENERATOR="Unix Makefiles"
  else
    echo "No build tool found (ninja/make). Install Ninja (recommended) or Make." >&2
    exit 1
  fi
fi

if [[ -z "${PREFIX}" ]]; then
  PREFIX="$(pwd)/install"
fi

if [[ "${GENERATOR}" == "Ninja" ]] && ! command -v ninja >/dev/null 2>&1; then
  TOOLS_DIR=".tools/ninja"
  NINJA_BIN="${TOOLS_DIR}/ninja"
  if [[ ! -x "${NINJA_BIN}" ]]; then
    mkdir -p "${TOOLS_DIR}"
    OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
    ARCH="$(uname -m)"
    URL=""
    if [[ "${OS}" == "linux" ]] && [[ "${ARCH}" == "x86_64" ]]; then
      URL="https://github.com/ninja-build/ninja/releases/latest/download/ninja-linux.zip"
    elif [[ "${OS}" == "darwin" ]] && [[ "${ARCH}" == "x86_64" || "${ARCH}" == "arm64" ]]; then
      URL="https://github.com/ninja-build/ninja/releases/latest/download/ninja-mac.zip"
    fi
    if [[ -z "${URL}" ]]; then
      echo "Ninja is required for generator Ninja; install Ninja using your package manager." >&2
      exit 1
    fi
    TMP_ZIP="$(mktemp)"
    if command -v curl >/dev/null 2>&1; then
      curl -fsSL "${URL}" -o "${TMP_ZIP}"
    elif command -v wget >/dev/null 2>&1; then
      wget -qO "${TMP_ZIP}" "${URL}"
    else
      echo "curl/wget not found. Install Ninja or curl/wget." >&2
      exit 1
    fi
    unzip -o "${TMP_ZIP}" -d "${TOOLS_DIR}" >/dev/null
    rm -f "${TMP_ZIP}"
    chmod +x "${NINJA_BIN}"
  fi
  export PATH="$(cd "${TOOLS_DIR}" && pwd):${PATH}"
fi

if [[ "${CLEAN}" -eq 1 ]]; then
  rm -rf "${BUILD_DIR}"
fi

CONFIG_ARGS=()
if [[ "${GENERATOR}" != *"Visual Studio"* ]] && [[ "${GENERATOR}" != "Ninja Multi-Config" ]]; then
  CONFIG_ARGS+=("-DCMAKE_BUILD_TYPE=${CONFIG}")
fi

PREFIX_ARGS=()
PREFIX_ARGS+=("-DCMAKE_INSTALL_PREFIX=${PREFIX}")

cmake -S . -B "${BUILD_DIR}" -G "${GENERATOR}" "${CONFIG_ARGS[@]}" "${PREFIX_ARGS[@]}"

BUILD_ARGS=()
if [[ -n "${PARALLEL}" ]]; then
  BUILD_ARGS+=("--parallel" "${PARALLEL}")
fi

cmake --build "${BUILD_DIR}" --config "${CONFIG}" "${BUILD_ARGS[@]}"
cmake --install "${BUILD_DIR}" --config "${CONFIG}"

if [[ "${PACKAGE}" -eq 1 ]]; then
  (cd "${BUILD_DIR}" && cpack -C "${CONFIG}")
fi

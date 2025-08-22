#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX=${INSTALL_PREFIX:-${BUILD_DIR}/install}
BUILD_JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

rm -rf "${BUILD_DIR}"

echo "==> Building RapidOcrOnnx (CLIB only)"
echo "    BUILD_TYPE=${BUILD_TYPE}"
cmake -S . -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}"
cmake --build "${BUILD_DIR}" -j"${BUILD_JOBS}"
cmake --install "${BUILD_DIR}"
echo "==> Done. Installed to ${INSTALL_PREFIX}"

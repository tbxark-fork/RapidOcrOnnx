#!/usr/bin/env bash
set -euo pipefail

# 简化版构建脚本：仅构建 CLIB (共享库)
# 可通过环境变量覆盖：BUILD_TYPE(默认Release)、OCR_ONNX(默认CPU)

BUILD_TYPE=${BUILD_TYPE:-Release}
OCR_ONNX=${OCR_ONNX:-CPU}

BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX=${INSTALL_PREFIX:-${BUILD_DIR}/install}

echo "==> Building RapidOcrOnnx (CLIB only)"
echo "    BUILD_TYPE=${BUILD_TYPE} OCR_ONNX=${OCR_ONNX}"

cmake -S . -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
  -DOCR_ONNX="${OCR_ONNX}"

cmake --build "${BUILD_DIR}" -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)"
cmake --install "${BUILD_DIR}"

echo "==> Done. Installed to ${INSTALL_PREFIX}"

#!/usr/bin/env bash
set -euo pipefail

IMAGE="quay.io/pypa/manylinux2014_x86_64"

echo "Running portable build inside ${IMAGE} container..."
docker run --rm -v "${PWD}":/io ${IMAGE} bash -lc '
  set -euo pipefail
  # enable newer toolchain if available
  if [ -f /opt/rh/devtoolset-10/enable ]; then \
    set +u; \
    source /opt/rh/devtoolset-10/enable; \
    set -u; \
  fi
  # ensure cmake command available
  if ! command -v cmake >/dev/null 2>&1 && command -v cmake3 >/dev/null 2>&1; then ln -sf /usr/bin/cmake3 /usr/bin/cmake; fi
  cmake --version
  cmake -S /io -B /io/build -DCMAKE_BUILD_TYPE=Release && \
  cmake --build /io/build -j"$(nproc)" && \
  cmake --install /io/build --prefix /io/build/install && \
  echo "==== ldd of installed lib ====" && \
  ldd /io/build/install/lib/libRapidOcrOnnx.so || true
'

echo "Done. Artifacts in build/install."

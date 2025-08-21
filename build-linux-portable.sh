#!/usr/bin/env bash
set -euo pipefail

docker pull dockcross/linux-x64
docker run --rm -v "$(pwd)":/work dockcross/linux-x64 /bin/bash -c './build.sh'
echo "==> Done. Artifacts are in build/install"

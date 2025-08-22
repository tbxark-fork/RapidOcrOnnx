#!/usr/bin/env bash
set -euo pipefail

docker pull dockcross/linux-x86
docker run --rm -v "$(pwd)":/work -w /work dockcross/linux-x86 /bin/bash -c './build.sh'
echo "==> Done. Artifacts are in build/install"

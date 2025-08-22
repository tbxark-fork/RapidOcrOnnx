#!/usr/bin/env bash
set -euo pipefail

docker pull rockylinux/rockylinux:9.6
docker run --rm -v "$(pwd)":/work rockylinux/rockylinux:9.6 /bin/bash -c './build.sh'
echo "==> Done. Artifacts are in build/install"

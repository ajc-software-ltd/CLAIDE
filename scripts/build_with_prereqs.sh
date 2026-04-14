#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BUILD_TYPE="${1:-Debug}"
RUN_TESTS="${2:-on}"

"${ROOT_DIR}/scripts/bootstrap_prereqs_ubuntu.sh"

cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

if [[ "${RUN_TESTS}" == "on" ]]; then
  ctest --test-dir "${BUILD_DIR}" --output-on-failure
fi

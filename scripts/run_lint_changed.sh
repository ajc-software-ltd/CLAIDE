#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build}"
CLANG_TIDY_BIN="${CLANG_TIDY_BIN:-clang-tidy}"
BASE_REF="${LINT_BASE_REF:-origin/main}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
  echo "[lint-fast] compile_commands.json not found in ${BUILD_DIR}." >&2
  exit 2
fi

if git -C "${ROOT_DIR}" rev-parse --verify "${BASE_REF}" >/dev/null 2>&1; then
  RANGE="$(git -C "${ROOT_DIR}" merge-base HEAD "${BASE_REF}")..HEAD"
else
  RANGE="HEAD~1..HEAD"
fi

mapfile -t CHANGED_CPP < <(git -C "${ROOT_DIR}" diff --name-only --diff-filter=ACMR "${RANGE}" | awk '/^(src|tests)\/.*\.cpp$/')
if [[ ${#CHANGED_CPP[@]} -eq 0 ]]; then
  echo "[lint-fast] No changed C++ translation units in ${RANGE}."
  exit 0
fi

echo "[lint-fast] Running clang-tidy on ${#CHANGED_CPP[@]} changed file(s)..."
"${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" -checks='-modernize-use-std-print,-bugprone-chained-comparison' "${CHANGED_CPP[@]}"

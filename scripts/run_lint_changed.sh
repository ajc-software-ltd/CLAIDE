#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build}"
CLANG_TIDY_BIN="${CLANG_TIDY_BIN:-clang-tidy}"
BASE_REF="${LINT_BASE_REF:-origin/main}"
SCOPE_FILTER="${LINT_SCOPE:-}"
REPORT_DIR="${BUILD_DIR}/reports"
RAW_REPORT="${REPORT_DIR}/clang-tidy.changed.raw.txt"
UNIQUE_REPORT="${REPORT_DIR}/clang-tidy.changed.unique.txt"
SUMMARY_REPORT="${REPORT_DIR}/clang-tidy.changed.summary.md"

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
if [[ -n "${SCOPE_FILTER}" ]]; then
  mapfile -t CHANGED_CPP < <(printf "%s\n" "${CHANGED_CPP[@]}" | grep -E "^${SCOPE_FILTER}" || true)
fi
if [[ ${#CHANGED_CPP[@]} -eq 0 ]]; then
  if [[ -n "${SCOPE_FILTER}" ]]; then
    echo "[lint-fast] No changed C++ translation units in ${RANGE} for scope '${SCOPE_FILTER}'."
  else
    echo "[lint-fast] No changed C++ translation units in ${RANGE}."
  fi
  exit 0
fi

echo "[lint-fast] Running clang-tidy on ${#CHANGED_CPP[@]} changed file(s)..."
mkdir -p "${REPORT_DIR}"
set +e
"${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" \
  -checks='-modernize-use-std-print,-bugprone-chained-comparison' \
  --extra-arg=-std=gnu++23 \
  "${CHANGED_CPP[@]}" | tee "${RAW_REPORT}"
CLANG_TIDY_EXIT=$?
set -e

python3 "${ROOT_DIR}/scripts/lint_unique_report.py" "${RAW_REPORT}" "${UNIQUE_REPORT}" "${SUMMARY_REPORT}" >/dev/null 2>&1 || true
echo "[lint-fast] Wrote reports:"
echo "  - ${RAW_REPORT}"
echo "  - ${UNIQUE_REPORT}"
echo "  - ${SUMMARY_REPORT}"
exit "${CLANG_TIDY_EXIT}"

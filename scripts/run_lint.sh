#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build}"
CLANG_TIDY_BIN="${CLANG_TIDY_BIN:-clang-tidy}"
CORE_CHECKS="${CORE_CHECKS:--modernize-use-std-print}"
TEST_CHECKS="${TEST_CHECKS:--modernize-use-std-print,-bugprone-chained-comparison}"

REPORT_DIR="${BUILD_DIR}/reports"
RAW_REPORT="${REPORT_DIR}/clang-tidy.raw.txt"
UNIQUE_REPORT="${REPORT_DIR}/clang-tidy.unique.txt"
SUMMARY_REPORT="${REPORT_DIR}/clang-tidy.summary.md"

mkdir -p "${REPORT_DIR}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
  echo "[lint] compile_commands.json not found in ${BUILD_DIR}. Configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON." >&2
  exit 2
fi

mapfile -t CORE_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^src\/.*\.cpp$/')
mapfile -t TEST_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^tests\/.*\.cpp$/')

if [[ ${#CORE_FILES[@]} -eq 0 ]]; then
  echo "[lint] No core .cpp files found." >&2
  exit 2
fi

echo "[lint] Running clang-tidy on core translation units (${#CORE_FILES[@]} files)..."
set +e
"${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" -checks="${CORE_CHECKS}" "${CORE_FILES[@]}" 2>&1 | tee "${RAW_REPORT}"
CORE_STATUS=${PIPESTATUS[0]}

if [[ ${#TEST_FILES[@]} -gt 0 ]]; then
  echo "[lint] Running clang-tidy on test translation units (${#TEST_FILES[@]} files)..."
  "${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" -checks="${TEST_CHECKS}" "${TEST_FILES[@]}" 2>&1 | tee -a "${RAW_REPORT}"
  TEST_STATUS=${PIPESTATUS[0]}
else
  TEST_STATUS=0
fi
set -e

python3 "${ROOT_DIR}/scripts/lint_unique_report.py" "${RAW_REPORT}" "${UNIQUE_REPORT}" "${SUMMARY_REPORT}"

if [[ ${CORE_STATUS} -ne 0 || ${TEST_STATUS} -ne 0 ]]; then
  echo "[lint] clang-tidy reported diagnostics/errors. See ${SUMMARY_REPORT}" >&2
  exit 1
fi

echo "[lint] clang-tidy completed successfully."

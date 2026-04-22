#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build}"
CLANG_TIDY_BIN="${CLANG_TIDY_BIN:-clang-tidy}"
CORE_CHECKS="${CORE_CHECKS:--modernize-use-std-print}"
TEST_CHECKS="${TEST_CHECKS:--modernize-use-std-print,-bugprone-chained-comparison}"
LINT_SCOPE="${LINT_SCOPE:-full}"
LINT_STD_ARG="${LINT_STD_ARG:---extra-arg=-std=gnu++23}"

REPORT_DIR="${BUILD_DIR}/reports"
REPORT_SCOPE="${LINT_SCOPE}"
if [[ "${LINT_SCOPE}" == "full" ]]; then
  REPORT_SCOPE=""
fi
RAW_REPORT="${REPORT_DIR}/clang-tidy${REPORT_SCOPE:+.${REPORT_SCOPE}}.raw.txt"
UNIQUE_REPORT="${REPORT_DIR}/clang-tidy${REPORT_SCOPE:+.${REPORT_SCOPE}}.unique.txt"
SUMMARY_REPORT="${REPORT_DIR}/clang-tidy${REPORT_SCOPE:+.${REPORT_SCOPE}}.summary.md"

mkdir -p "${REPORT_DIR}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
  echo "[lint] compile_commands.json not found in ${BUILD_DIR}. Configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON." >&2
  exit 2
fi

case "${LINT_SCOPE}" in
  full)
    mapfile -t CORE_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^src\/.*\.cpp$/')
    mapfile -t TEST_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^tests\/.*\.cpp$/')
    ;;
  core)
    mapfile -t CORE_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^(src\/core|src\/platform)\/.*\.cpp$/')
    TEST_FILES=()
    ;;
  ui)
    mapfile -t CORE_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^src\/ui\/.*\.cpp$/')
    TEST_FILES=()
    ;;
  vulkan)
    mapfile -t CORE_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^(src\/vulkan|src\/render|src\/core\/runtime)\/.*\.cpp$/')
    TEST_FILES=()
    ;;
  tests)
    CORE_FILES=()
    mapfile -t TEST_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^tests\/.*\.cpp$/')
    ;;
  *)
    echo "[lint] Unsupported LINT_SCOPE='${LINT_SCOPE}'. Expected one of: full, core, ui, vulkan, tests." >&2
    exit 2
    ;;
esac

if [[ ${#CORE_FILES[@]} -eq 0 && ${#TEST_FILES[@]} -eq 0 ]]; then
  echo "[lint] No .cpp files found for scope '${LINT_SCOPE}'." >&2
  exit 2
fi

set +e
if [[ ${#CORE_FILES[@]} -gt 0 ]]; then
  echo "[lint] Running clang-tidy on scope '${LINT_SCOPE}' core translation units (${#CORE_FILES[@]} files)..."
  "${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" -checks="${CORE_CHECKS}" "${LINT_STD_ARG}" "${CORE_FILES[@]}" 2>&1 | tee "${RAW_REPORT}"
  CORE_STATUS=${PIPESTATUS[0]}
else
  : > "${RAW_REPORT}"
  CORE_STATUS=0
fi

if [[ ${#TEST_FILES[@]} -gt 0 ]]; then
  echo "[lint] Running clang-tidy on scope '${LINT_SCOPE}' test translation units (${#TEST_FILES[@]} files)..."
  "${CLANG_TIDY_BIN}" -p "${BUILD_DIR}" -checks="${TEST_CHECKS}" "${LINT_STD_ARG}" "${TEST_FILES[@]}" 2>&1 | tee -a "${RAW_REPORT}"
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

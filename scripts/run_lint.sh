#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build}"
CLANG_TIDY_BIN="${CLANG_TIDY_BIN:-clang-tidy}"
CORE_CHECKS="${CORE_CHECKS:--modernize-use-std-print}"
TEST_CHECKS="${TEST_CHECKS:--modernize-use-std-print,-bugprone-chained-comparison}"
LINT_MAX_BUCKET_A="${LINT_MAX_BUCKET_A:-0}"
LINT_MAX_SRC_BUCKET_B="${LINT_MAX_SRC_BUCKET_B:-24}"

REPORT_DIR="${BUILD_DIR}/reports"
RAW_REPORT="${REPORT_DIR}/clang-tidy.raw.txt"
UNIQUE_REPORT="${REPORT_DIR}/clang-tidy.unique.txt"
SUMMARY_REPORT="${REPORT_DIR}/clang-tidy.summary.md"

mkdir -p "${REPORT_DIR}"

if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
  echo "[lint] compile_commands.json not found in ${BUILD_DIR}. Configure with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON." >&2
  exit 2
fi

if ! "${CLANG_TIDY_BIN}" --version >/dev/null 2>&1; then
  echo "[lint] clang-tidy binary '${CLANG_TIDY_BIN}' is not executable." >&2
  exit 2
fi

CLANG_TIDY_VERSION="$("${CLANG_TIDY_BIN}" --version | sed -nE 's/.*version ([0-9]+)\..*/\1/p' | head -n 1)"
if [[ -n "${CLANG_TIDY_VERSION}" && "${CLANG_TIDY_VERSION}" -lt 20 ]]; then
  echo "[lint] clang-tidy ${CLANG_TIDY_VERSION} detected; version 20+ is required to avoid known false parser/tool failures." >&2
  exit 2
fi

mapfile -t CORE_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^src\/.*\.cpp$/')
mapfile -t TEST_FILES < <(cd "${ROOT_DIR}" && git ls-files | awk '/^tests\/.*\.cpp$/')

if [[ ${#CORE_FILES[@]} -eq 0 ]]; then
  echo "[lint] No core .cpp files found." >&2
  exit 2
fi

FILTERED_DB_DIR="${BUILD_DIR}/lint"
FILTERED_DB_PATH="${FILTERED_DB_DIR}/compile_commands.json"
mkdir -p "${FILTERED_DB_DIR}"

python3 - <<'PY' "${BUILD_DIR}/compile_commands.json" "${FILTERED_DB_PATH}" "${ROOT_DIR}"
import json
import pathlib
import sys

source_path = pathlib.Path(sys.argv[1])
out_path = pathlib.Path(sys.argv[2])
root = pathlib.Path(sys.argv[3]).resolve()

with source_path.open("r", encoding="utf-8") as handle:
    commands = json.load(handle)

chosen = {}
for entry in commands:
    file_path = pathlib.Path(entry["file"]).resolve()
    key = str(file_path)
    command_text = entry.get("command", "")
    if key not in chosen:
        chosen[key] = entry
        continue

    existing = chosen[key]
    existing_command = existing.get("command", "")

    relative = file_path.relative_to(root)
    is_test_source = "tests" in relative.parts
    prefer_tests = is_test_source and "CLIADETests" in command_text and "CLIADETests" not in existing_command
    prefer_app = (not is_test_source) and "CLIADETests" in existing_command and "CLIADETests" not in command_text

    if prefer_tests or prefer_app:
        chosen[key] = entry

filtered = list(chosen.values())
out_path.write_text(json.dumps(filtered, indent=2), encoding="utf-8")
PY

echo "[lint] Running clang-tidy on core translation units (${#CORE_FILES[@]} files)..."
set +e
"${CLANG_TIDY_BIN}" -p "${FILTERED_DB_DIR}" -checks="${CORE_CHECKS}" "${CORE_FILES[@]}" 2>&1 | tee "${RAW_REPORT}"
CORE_STATUS=${PIPESTATUS[0]}

if [[ ${#TEST_FILES[@]} -gt 0 ]]; then
  echo "[lint] Running clang-tidy on test translation units (${#TEST_FILES[@]} files)..."
  "${CLANG_TIDY_BIN}" -p "${FILTERED_DB_DIR}" -checks="${TEST_CHECKS}" "${TEST_FILES[@]}" 2>&1 | tee -a "${RAW_REPORT}"
  TEST_STATUS=${PIPESTATUS[0]}
else
  TEST_STATUS=0
fi
set -e

python3 "${ROOT_DIR}/scripts/lint_unique_report.py" \
  "${RAW_REPORT}" \
  "${UNIQUE_REPORT}" \
  "${SUMMARY_REPORT}" \
  "${LINT_MAX_BUCKET_A}" \
  "${LINT_MAX_SRC_BUCKET_B}"

if [[ ${CORE_STATUS} -ne 0 || ${TEST_STATUS} -ne 0 ]]; then
  echo "[lint] clang-tidy reported diagnostics/errors. See ${SUMMARY_REPORT}" >&2
  echo "[lint] Active thresholds: LINT_MAX_BUCKET_A=${LINT_MAX_BUCKET_A}, LINT_MAX_SRC_BUCKET_B=${LINT_MAX_SRC_BUCKET_B}" >&2
  exit 1
fi

echo "[lint] clang-tidy completed successfully."

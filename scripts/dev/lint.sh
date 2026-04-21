#!/usr/bin/env bash
set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"

usage() {
    cat <<'USAGE'
Usage: scripts/dev/lint.sh [--preset <dev|dev-make>] <mode>

Modes:
  full         -> lint
  fast         -> lint-fast
  core         -> lint-core
  ui           -> lint-ui
  vulkan       -> lint-vulkan
  tests        -> lint-tests
  format-check -> format-check
  validate     -> validate
USAGE
}

preset=""
mode=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --preset)
            [[ $# -ge 2 ]] || { error "--preset requires a value."; exit 2; }
            preset="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            if [[ -z "${mode}" ]]; then
                mode="$1"
                shift
            else
                error "Unexpected argument: $1"
                usage
                exit 2
            fi
            ;;
    esac
done

if [[ -z "${mode}" ]]; then
    error "Lint mode is required."
    usage
    exit 2
fi

if [[ -z "${preset}" ]]; then
    preset="$(resolve_active_preset)"
fi

build_dir="$(preset_build_dir "${preset}")"
ensure_configured "${build_dir}"

case "${mode}" in
    full) target="lint" ;;
    fast) target="lint-fast" ;;
    core) target="lint-core" ;;
    ui) target="lint-ui" ;;
    vulkan) target="lint-vulkan" ;;
    tests) target="lint-tests" ;;
    format-check) target="format-check" ;;
    validate) target="validate" ;;
    *)
        error "Unsupported lint mode '${mode}'."
        usage
        exit 2
        ;;
esac

if ! cmake_target_exists "${build_dir}" "${target}"; then
    error "CMake target '${target}' is not available in ${build_dir}."
    error "Available safe modes here are: full, fast, format-check, validate (if configured)."
    exit 4
fi

info "Running lint mode '${mode}' (${target})"
cmake --build "${build_dir}" --target "${target}"

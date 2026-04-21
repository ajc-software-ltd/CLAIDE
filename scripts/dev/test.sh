#!/usr/bin/env bash
set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"

usage() {
    cat <<'USAGE'
Usage: scripts/dev/test.sh [--preset <dev|dev-make>] [-R <regex>]

Runs ctest with --output-on-failure against the active preset build directory.
USAGE
}

preset=""
regex=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --preset)
            [[ $# -ge 2 ]] || { error "--preset requires a value."; exit 2; }
            preset="$2"
            shift 2
            ;;
        -R)
            [[ $# -ge 2 ]] || { error "-R requires a regex value."; exit 2; }
            regex="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            error "Unknown argument: $1"
            usage
            exit 2
            ;;
    esac
done

if [[ -z "${preset}" ]]; then
    preset="$(resolve_active_preset)"
fi

build_dir="$(preset_build_dir "${preset}")"
ensure_configured "${build_dir}"

cmd=(ctest --test-dir "${build_dir}" --output-on-failure)
if [[ -n "${regex}" ]]; then
    cmd+=(-R "${regex}")
fi

info "Running tests (${preset} -> ${build_dir})"
"${cmd[@]}"

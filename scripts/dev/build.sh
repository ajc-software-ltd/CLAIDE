#!/usr/bin/env bash
set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"

usage() {
    cat <<'USAGE'
Usage: scripts/dev/build.sh [--preset <dev|dev-make>] [--target <name>] [-- <extra build args...>]

Builds CLAIDE from the active preset build directory.
USAGE
}

preset=""
target=""
extra_args=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --preset)
            [[ $# -ge 2 ]] || { error "--preset requires a value."; exit 2; }
            preset="$2"
            shift 2
            ;;
        --target)
            [[ $# -ge 2 ]] || { error "--target requires a value."; exit 2; }
            target="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        --)
            shift
            extra_args=("$@")
            break
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

cmd=(cmake --build "${build_dir}")
if [[ -n "${target}" ]]; then
    cmd+=(--target "${target}")
fi
if [[ ${#extra_args[@]} -gt 0 ]]; then
    cmd+=(-- "${extra_args[@]}")
fi

info "Building (${preset} -> ${build_dir})"
"${cmd[@]}"

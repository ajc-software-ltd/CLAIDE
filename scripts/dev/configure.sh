#!/usr/bin/env bash
set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/common.sh"

usage() {
    cat <<'USAGE'
Usage: scripts/dev/configure.sh [--clean] [--preset <dev|dev-make>] [-- <extra cmake args...>]

Configures CLAIDE using CMake presets.
Defaults to 'dev' and automatically falls back to 'dev-make' when Ninja is unavailable.
USAGE
}

clean=0
preset=""
extra_args=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)
            clean=1
            shift
            ;;
        --preset)
            [[ $# -ge 2 ]] || { error "--preset requires a value."; exit 2; }
            preset="$2"
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
    preset="$(default_preset)"
fi

build_dir="$(preset_build_dir "${preset}")"

if [[ ${clean} -eq 1 ]]; then
    info "Removing ${build_dir}"
    rm -rf "${build_dir}"
fi

info "Configuring preset '${preset}'"
cmake --preset "${preset}" "${extra_args[@]}"
set_active_preset "${preset}"

if cmake_target_exists "${build_dir}" "refresh-compile-commands"; then
    info "Refreshing compile_commands.json"
    cmake --build "${build_dir}" --target refresh-compile-commands
else
    info "Target refresh-compile-commands not found; skipping compile_commands sync"
fi

info "Configure complete (${preset} -> ${build_dir})"

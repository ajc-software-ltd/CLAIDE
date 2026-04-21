#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
STATE_FILE="${REPO_ROOT}/.codex-dev-preset"

info() {
    echo "[dev] $*"
}

warn() {
    echo "[dev] warning: $*" >&2
}

error() {
    echo "[dev] error: $*" >&2
}

has_ninja() {
    command -v ninja >/dev/null 2>&1
}

default_preset() {
    if has_ninja; then
        echo "dev"
    else
        echo "dev-make"
    fi
}

preset_build_dir() {
    case "$1" in
        dev)
            echo "${REPO_ROOT}/build"
            ;;
        dev-make)
            echo "${REPO_ROOT}/build-make"
            ;;
        *)
            error "Unsupported preset '$1'. Expected: dev or dev-make."
            return 2
            ;;
    esac
}

resolve_active_preset() {
    if [[ -f "${STATE_FILE}" ]]; then
        local preset
        preset="$(<"${STATE_FILE}")"
        if [[ "${preset}" == "dev" || "${preset}" == "dev-make" ]]; then
            echo "${preset}"
            return 0
        fi
    fi

    default_preset
}

set_active_preset() {
    local preset="$1"
    printf '%s\n' "${preset}" > "${STATE_FILE}"
}

ensure_configured() {
    local build_dir="$1"
    if [[ ! -f "${build_dir}/CMakeCache.txt" ]]; then
        error "Build directory '${build_dir}' is not configured. Run scripts/dev/configure.sh first."
        return 3
    fi
}

cmake_target_exists() {
    local build_dir="$1"
    local target="$2"
    cmake --build "${build_dir}" --target help 2>/dev/null | grep -Eq "(^|[[:space:]])${target}($|[[:space:]])"
}

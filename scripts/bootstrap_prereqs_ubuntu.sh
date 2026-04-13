#!/usr/bin/env bash
set -euo pipefail

if [[ "${EUID}" -ne 0 ]]; then
  echo "[bootstrap] Re-running with sudo..."
  exec sudo -E bash "$0" "$@"
fi

if [[ ! -f /etc/os-release ]]; then
  echo "[bootstrap] /etc/os-release not found; cannot detect distro." >&2
  exit 1
fi

# shellcheck disable=SC1091
source /etc/os-release
if [[ "${ID:-}" != "ubuntu" && "${ID_LIKE:-}" != *"debian"* ]]; then
  echo "[bootstrap] Unsupported distro: ${ID:-unknown}. This script supports Ubuntu/Debian." >&2
  exit 1
fi

packages=(
  build-essential
  cmake
  pkg-config
  libwxgtk3.2-dev
  libspdlog-dev
  catch2
  libmagick++-dev
  libvulkan-dev
  glslang-tools
)

missing=()
for pkg in "${packages[@]}"; do
  if ! dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed"; then
    missing+=("$pkg")
  fi
done

if [[ ${#missing[@]} -eq 0 ]]; then
  echo "[bootstrap] All required packages are already installed."
  exit 0
fi

echo "[bootstrap] Installing missing packages: ${missing[*]}"
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y "${missing[@]}"

echo "[bootstrap] Prerequisites installed successfully."

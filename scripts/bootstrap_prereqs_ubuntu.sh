#!/usr/bin/env bash
set -euo pipefail

readonly k_marker_path="/var/tmp/claide-bootstrap.ok"
readonly k_apt_update_stamp="/var/tmp/claide-apt-update.stamp"
readonly k_apt_update_max_age_seconds=86400

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
  ccache
  clang-tidy-20
  ninja-build
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
  echo "[bootstrap] All required packages are already installed (cached)."
  {
    echo "status=ok"
    echo "updated_at=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    printf 'packages=%s\n' "${packages[*]}"
  } > "${k_marker_path}"
  exit 0
fi

needs_apt_update=1
if [[ -f "${k_apt_update_stamp}" ]]; then
  now_epoch=$(date +%s)
  last_epoch=$(stat -c %Y "${k_apt_update_stamp}")
  age_seconds=$((now_epoch - last_epoch))
  if (( age_seconds < k_apt_update_max_age_seconds )); then
    needs_apt_update=0
  fi
fi

if (( needs_apt_update == 1 )); then
  echo "[bootstrap] Refreshing apt metadata..."
  apt-get update
  touch "${k_apt_update_stamp}"
else
  echo "[bootstrap] Skipping apt metadata refresh (cached <24h)."
fi

echo "[bootstrap] Installing missing packages: ${missing[*]}"
DEBIAN_FRONTEND=noninteractive apt-get install -y "${missing[@]}"

echo "[bootstrap] Verifying toolchain..."
command -v cmake >/dev/null
command -v ccache >/dev/null
command -v clang-tidy-20 >/dev/null
command -v ninja >/dev/null
command -v glslangValidator >/dev/null
command -v ctest >/dev/null

if [[ ! -f /usr/include/vulkan/vulkan.h ]]; then
  echo "[bootstrap] Vulkan headers not found after install." >&2
  exit 1
fi

{
  echo "status=ok"
  echo "updated_at=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  printf 'packages=%s\n' "${packages[*]}"
} > "${k_marker_path}"

echo "[bootstrap] Prerequisites installed and verified successfully."

#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
yocto_build_dir="${VEHICLE_NODE_YOCTO_BUILD_DIR:-$HOME/vehicle-node-yocto-build}"
layer_dir="$project_root/yocto/meta-vehicle-node"

echo "Yocto build directory: $yocto_build_dir"

if [[ -z "${POKY_DIR:-}" || ! -d "${POKY_DIR:-}" ]]; then
    echo "POKY_DIR is not set or does not exist."
    echo "Install or clone Poky separately, then run:"
    echo "export POKY_DIR=~/poky"
    exit 1
fi

poky_dir="$(cd "$POKY_DIR" && pwd)"
if [[ ! -f "$poky_dir/oe-init-build-env" ]]; then
    echo "Poky initialization script not found: $poky_dir/oe-init-build-env" >&2
    exit 1
fi

mkdir -p "$yocto_build_dir"
yocto_build_dir="$(cd "$yocto_build_dir" && pwd)"
case "$yocto_build_dir/" in
    "$project_root/"*)
        echo "Yocto build directory must be outside the project repository." >&2
        exit 1
        ;;
esac

# shellcheck disable=SC1090
source "$poky_dir/oe-init-build-env" "$yocto_build_dir"

if ! command -v bitbake-layers >/dev/null 2>&1; then
    echo "bitbake-layers was not provided by the Poky environment." >&2
    exit 1
fi
if ! bitbake-layers show-layers 2>/dev/null | grep -Fq "$layer_dir"; then
    bitbake-layers add-layer "$layer_dir"
fi

settings_marker="# vehicle-node project settings"
if ! grep -Fqx "$settings_marker" conf/local.conf; then
    cat >> conf/local.conf <<'SETTINGS'

# vehicle-node project settings
MACHINE ?= "qemux86-64"
DISTRO_FEATURES:append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts = ""
SETTINGS
fi

bitbake vehicle-node-image

echo "Yocto build directory: $yocto_build_dir"
echo "Image deploy directory: $yocto_build_dir/tmp/deploy/images/qemux86-64"
echo "Run QEMU with:"
echo "./scripts/run_yocto_qemu.sh"

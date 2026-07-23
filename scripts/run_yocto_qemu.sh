#!/usr/bin/env bash
set -euo pipefail

yocto_build_dir="${VEHICLE_NODE_YOCTO_BUILD_DIR:-$HOME/vehicle-node-yocto-build}"

echo "Yocto build directory: $yocto_build_dir"

if [[ -z "${POKY_DIR:-}" || ! -d "${POKY_DIR:-}" ]]; then
    echo "POKY_DIR is not set or does not exist."
    echo "Install or clone Poky separately, then run:"
    echo "export POKY_DIR=~/poky"
    exit 1
fi

poky_dir="$(cd "$POKY_DIR" && pwd)"
deploy_dir="$yocto_build_dir/tmp/deploy/images/qemux86-64"
shopt -s nullglob
image_files=(
    "$deploy_dir"/vehicle-node-image-qemux86-64*.qemuboot.conf
    "$deploy_dir"/vehicle-node-image-qemux86-64*.wic
    "$deploy_dir"/vehicle-node-image-qemux86-64*.ext4
)
shopt -u nullglob

if [[ ! -d "$yocto_build_dir" || ${#image_files[@]} -eq 0 ]]; then
    echo "Yocto image not found. Run ./scripts/build_yocto.sh first."
    exit 1
fi
if [[ ! -f "$poky_dir/oe-init-build-env" ]]; then
    echo "Poky initialization script not found: $poky_dir/oe-init-build-env" >&2
    exit 1
fi

echo "After the target boots, test with:"
cat <<'COMMANDS'
  systemctl status vehicle-node
  vehicle-node --paths
  vehicle-node --status
  vehicle-node --health
  vehicle-node --check-can
  vehicle-node --config
  vehicle-node --version
  journalctl -u vehicle-node --no-pager
  cat /etc/vehicle-node/vehicle-node.yaml
  cat /var/lib/vehicle-node/status.json
  tail -n 20 /var/log/vehicle-node/vehicle-node.log
COMMANDS

# shellcheck disable=SC1090
source "$poky_dir/oe-init-build-env" "$yocto_build_dir"
runqemu qemux86-64 vehicle-node-image nographic

#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_dir="${VEHICLE_NODE_BUILDROOT_OUTPUT:-$HOME/vehicle-node-buildroot-output/buildroot-qemu-x86_64}"
images_dir="$output_dir/images"
start_script="$images_dir/start-qemu.sh"

echo "Buildroot output: $output_dir"

if [[ ! -d "$images_dir" ]]; then
    echo "Buildroot images were not found: $images_dir" >&2
    echo "Run ./scripts/build_buildroot.sh first." >&2
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

if [[ -f "$start_script" ]]; then
    echo "Starting QEMU with Buildroot's generated script..."
    if [[ -x "$start_script" ]]; then
        exec "$start_script"
    else
        exec bash "$start_script"
    fi
fi

kernel="$images_dir/bzImage"
rootfs="$images_dir/rootfs.ext2"
if [[ ! -f "$rootfs" && -f "$images_dir/rootfs.ext4" ]]; then
    rootfs="$images_dir/rootfs.ext4"
fi

if [[ ! -f "$kernel" || ! -f "$rootfs" ]]; then
    echo "Required QEMU images are missing (bzImage and rootfs.ext2/rootfs.ext4)." >&2
    echo "Run ./scripts/build_buildroot.sh first." >&2
    exit 1
fi
if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "qemu-system-x86_64 is not installed or not on PATH." >&2
    exit 1
fi

echo "Starting QEMU directly; press Ctrl-a x to exit."
exec qemu-system-x86_64 \
    -M pc \
    -m 512M \
    -kernel "$kernel" \
    -drive "file=$rootfs,if=virtio,format=raw" \
    -append "root=/dev/vda console=ttyS0 rw" \
    -netdev user,id=net0 \
    -device virtio-net-pci,netdev=net0 \
    -nographic

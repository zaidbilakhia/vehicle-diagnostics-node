#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_dir="${VEHICLE_NODE_BUILDROOT_OUTPUT:-$HOME/vehicle-node-buildroot-output/buildroot-qemu-x86_64}"
external_dir="$project_root/buildroot/external-vehicle-node"

echo "Buildroot output: $output_dir"

if [[ -z "${BUILDROOT_DIR:-}" || ! -d "${BUILDROOT_DIR:-}" ]]; then
    echo "BUILDROOT_DIR is not set or does not exist."
    echo "Install or clone Buildroot separately, then run:"
    echo "export BUILDROOT_DIR=~/buildroot"
    exit 1
fi

buildroot_dir="$(cd "$BUILDROOT_DIR" && pwd)"
mkdir -p "$output_dir"
output_dir="$(cd "$output_dir" && pwd)"

case "$output_dir/" in
    "$project_root/"*)
        echo "Buildroot output must be outside the project repository." >&2
        exit 1
        ;;
esac

echo "Configuring Buildroot with the vehicle-node external defconfig..."
if ! make -C "$buildroot_dir" O="$output_dir" BR2_EXTERNAL="$external_dir" \
    vehicle_node_qemu_x86_64_defconfig; then
    echo "External defconfig failed; falling back to qemu_x86_64_defconfig."
    make -C "$buildroot_dir" O="$output_dir" BR2_EXTERNAL="$external_dir" \
        qemu_x86_64_defconfig
fi

config_tool="$buildroot_dir/utils/config"
if [[ -x "$config_tool" ]]; then
    "$config_tool" --file "$output_dir/.config" \
        -e BR2_TOOLCHAIN_BUILDROOT_WCHAR \
        -e BR2_TOOLCHAIN_BUILDROOT_CXX \
        -e BR2_INIT_SYSTEMD \
        -e BR2_PACKAGE_VEHICLE_NODE \
        -e BR2_LINUX_KERNEL \
        -e BR2_TARGET_ROOTFS_EXT2 \
        --set-str BR2_TARGET_ROOTFS_EXT2_SIZE "120M"

    if [[ -f "$buildroot_dir/package/iproute2/Config.in" ]] &&
       grep -q '^config BR2_PACKAGE_IPROUTE2$' "$buildroot_dir/package/iproute2/Config.in"; then
        "$config_tool" --file "$output_dir/.config" -e BR2_PACKAGE_IPROUTE2
    fi
    if [[ -f "$buildroot_dir/package/can-utils/Config.in" ]] &&
       grep -q '^config BR2_PACKAGE_CAN_UTILS$' "$buildroot_dir/package/can-utils/Config.in"; then
        "$config_tool" --file "$output_dir/.config" -e BR2_PACKAGE_CAN_UTILS
    fi
else
    echo "[WARN] Buildroot utils/config not found; using defconfig values as-is."
fi

make -C "$buildroot_dir" O="$output_dir" BR2_EXTERNAL="$external_dir" olddefconfig
make -C "$buildroot_dir" O="$output_dir" BR2_EXTERNAL="$external_dir"

echo "Buildroot output: $output_dir"
echo "Kernel: $output_dir/images/bzImage"
echo "Rootfs: $output_dir/images/rootfs.ext2"
echo "QEMU script: $output_dir/images/start-qemu.sh"

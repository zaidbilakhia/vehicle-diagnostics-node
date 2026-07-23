#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
external_dir="$project_root/buildroot/external-vehicle-node"
package_dir="$external_dir/package/vehicle-node"
output_dir="${VEHICLE_NODE_BUILDROOT_OUTPUT:-$HOME/vehicle-node-buildroot-output/buildroot-qemu-x86_64}"
failures=0

echo "Buildroot output: $output_dir"

check_path() {
    local label="$1"
    local path="$2"
    local kind="${3:-file}"

    if [[ "$kind" == "directory" && -d "$path" ]] ||
       [[ "$kind" == "file" && -f "$path" ]] ||
       [[ "$kind" == "executable" && -x "$path" ]] ||
       [[ "$kind" == "symlink" && -L "$path" ]]; then
        echo "[OK] $label"
    else
        echo "[FAIL] $label: $path"
        failures=$((failures + 1))
    fi
}

check_content() {
    local label="$1"
    local path="$2"
    local pattern="$3"

    if [[ -f "$path" ]] && grep -Fq "$pattern" "$path"; then
        echo "[OK] $label"
    else
        echo "[FAIL] $label"
        failures=$((failures + 1))
    fi
}

check_one_of() {
    local label="$1"
    shift
    local candidate
    for candidate in "$@"; do
        if [[ -f "$candidate" ]]; then
            echo "[OK] $label"
            return
        fi
    done
    echo "[FAIL] $label: none of the expected files exist"
    failures=$((failures + 1))
}

check_path "Buildroot external tree" "$external_dir" directory
check_path "external.desc" "$external_dir/external.desc"
check_path "external.mk" "$external_dir/external.mk"
check_path "external Config.in" "$external_dir/Config.in"
check_path "vehicle-node package config" "$package_dir/Config.in"
check_path "vehicle-node package makefile" "$package_dir/vehicle-node.mk"
check_path "vehicle-node target config" "$package_dir/files/vehicle-node.yaml"
check_path "vehicle-node target service" "$package_dir/files/vehicle-node.service"

check_content "BR2 package symbol" "$package_dir/Config.in" "BR2_PACKAGE_VEHICLE_NODE"
check_content "Package installs vehicle-node config" "$package_dir/vehicle-node.mk" "vehicle-node.yaml"
check_content "Package installs vehicle-node service" "$package_dir/vehicle-node.mk" "vehicle-node.service"
check_content "Service starts vehicle-node" "$package_dir/files/vehicle-node.service" "/usr/bin/vehicle-node --service"

if [[ ! -d "$output_dir" ]]; then
    echo "[INFO] Buildroot output not found yet. Run ./scripts/build_buildroot.sh first."
else
    check_path "Buildroot target binary" "$output_dir/target/usr/bin/vehicle-node" executable
    check_path "Buildroot target config" "$output_dir/target/etc/vehicle-node/vehicle-node.yaml"
    check_path "Buildroot target service" "$output_dir/target/usr/lib/systemd/system/vehicle-node.service"
    check_path "systemd enable symlink" \
        "$output_dir/target/etc/systemd/system/multi-user.target.wants/vehicle-node.service" symlink
    check_one_of "kernel image" \
        "$output_dir/images/bzImage" "$output_dir/images/Image" "$output_dir/images/vmlinuz"
    check_one_of "rootfs image" \
        "$output_dir/images/rootfs.ext2" "$output_dir/images/rootfs.ext4"
fi

if (( failures > 0 )); then
    echo "Buildroot verification failed with $failures issue(s)." >&2
    exit 1
fi

echo "Buildroot vehicle-node verification passed."

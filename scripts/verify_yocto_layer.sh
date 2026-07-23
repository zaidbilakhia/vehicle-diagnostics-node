#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
layer_dir="$project_root/yocto/meta-vehicle-node"
recipe="$layer_dir/recipes-apps/vehicle-node/vehicle-node_1.0.bb"
service="$layer_dir/recipes-apps/vehicle-node/files/vehicle-node.service"
config="$layer_dir/recipes-apps/vehicle-node/files/vehicle-node.yaml"
image="$layer_dir/recipes-core/images/vehicle-node-image.bb"
failures=0

check_path() {
    local label="$1"
    local path="$2"
    local kind="${3:-file}"

    if [[ "$kind" == "directory" && -d "$path" ]] ||
       [[ "$kind" == "file" && -f "$path" ]]; then
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

    if [[ -f "$path" ]] && grep -Eq "$pattern" "$path"; then
        echo "[OK] $label"
    else
        echo "[FAIL] $label"
        failures=$((failures + 1))
    fi
}

check_path "Yocto vehicle-node layer" "$layer_dir" directory
check_path "layer.conf" "$layer_dir/conf/layer.conf"
check_path "vehicle-node recipe" "$recipe"
check_path "vehicle-node service file" "$service"
check_path "vehicle-node target config" "$config"
check_path "vehicle-node image recipe" "$image"
check_path "MIT license" "$project_root/LICENSE"
check_path "Yocto documentation" "$project_root/docs/yocto.md"
check_path "Interview notes" "$project_root/docs/interview-notes.md"

check_content "README Milestone 3 section" "$project_root/README.md" 'Milestone 3.*Yocto'
check_content "Layer collection" "$layer_dir/conf/layer.conf" 'BBFILE_COLLECTIONS.*vehicle-node'
check_content "Recipe inherits CMake" "$recipe" '^inherit .*cmake'
check_content "Recipe inherits systemd" "$recipe" '^inherit .*systemd'
check_content "Recipe uses externalsrc" "$recipe" '^inherit .*externalsrc'
check_content "Recipe declares SYSTEMD_SERVICE" "$recipe" 'SYSTEMD_SERVICE'
check_content "Recipe enables the service" "$recipe" 'SYSTEMD_AUTO_ENABLE.*enable'
check_content "Recipe installs vehicle-node.service" "$recipe" 'vehicle-node\.service'
check_content "Recipe installs vehicle-node.yaml" "$recipe" 'vehicle-node\.yaml'
check_content "Image installs vehicle-node" "$image" 'IMAGE_INSTALL.*vehicle-node'
check_content "Image recipe name documented" "$project_root/docs/yocto.md" 'vehicle-node-image'

legacy_ref_pattern='au''dio-node|AU''DIO_NODE'
if [[ -d "$layer_dir" ]] && grep -RqiE "$legacy_ref_pattern" "$layer_dir"; then
    echo "[FAIL] Active Yocto layer still contains legacy naming references"
    failures=$((failures + 1))
else
    echo "[OK] No legacy naming references in active Yocto layer"
fi

if [[ -f "$project_root/LICENSE" && -f "$recipe" ]]; then
    actual_md5="$(md5sum "$project_root/LICENSE" | awk '{print $1}')"
    if grep -Fq "file://LICENSE;md5=$actual_md5" "$recipe"; then
        echo "[OK] LICENSE checksum"
    else
        echo "[FAIL] LICENSE checksum"
        failures=$((failures + 1))
    fi
fi

if (( failures > 0 )); then
    echo "Yocto vehicle layer verification failed with $failures issue(s)." >&2
    exit 1
fi

echo "Yocto vehicle layer verification passed."

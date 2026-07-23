#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$project_root"
./scripts/build.sh

user_unit_dir="$HOME/.config/systemd/user"
user_unit="$user_unit_dir/vehicle-node.service"
mkdir -p "$user_unit_dir"

sed \
    -e "s|ExecStart=/usr/bin/vehicle-node --service|ExecStart=$project_root/build/vehicle-node --service|" \
    -e "/Type=simple/a WorkingDirectory=$project_root\\nEnvironment=VEHICLE_NODE_CONFIG=$project_root/config/vehicle-node.yaml\\nEnvironment=VEHICLE_NODE_RUNTIME_DIR=$project_root/runtime" \
    -e "s|WantedBy=multi-user.target|WantedBy=default.target|" \
    systemd/vehicle-node.service > "$user_unit"

systemctl --user daemon-reload
systemctl --user enable vehicle-node.service
systemctl --user restart vehicle-node.service
systemctl --user status vehicle-node.service --no-pager

echo "Useful commands:"
echo "  journalctl --user -u vehicle-node.service -f"
echo "  systemctl --user stop vehicle-node.service"
echo "  systemctl --user status vehicle-node.service"

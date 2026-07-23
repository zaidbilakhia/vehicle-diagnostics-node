#!/usr/bin/env bash
set -euo pipefail

./scripts/build.sh
./build/vehicle-node --version
./build/vehicle-node --paths
./build/vehicle-node --config
./build/vehicle-node --check-can
./build/vehicle-node --health
./build/vehicle-node --status
timeout 8s ./build/vehicle-node --service || true

ls -l runtime
cat runtime/status.json
tail -n 10 runtime/vehicle-node.log

# Embedded Linux Vehicle Diagnostics Node

`vehicle-node` is a compact C++17 automotive Linux service. It reads target-specific configuration, checks Linux health and SocketCAN/vCAN availability, writes JSON status and timestamped logs, and runs continuously under systemd.

## Final architecture

```text
C++17 vehicle-node application
        ↓
Local Linux + systemd validation
        ↓
Buildroot package + QEMU embedded target
        ↓
Yocto layer + BitBake recipe
```

## Project status

| Milestone | Scope | Status |
|---|---|---|
| Milestone 1 | C++17 `vehicle-node` and local systemd | Complete |
| Milestone 2 | Buildroot and QEMU vehicle target | Complete and QEMU-verified |
| Milestone 3 | Yocto vehicle layer | Complete at Yocto layer/repository level |

Full Yocto image building is optional and requires an external Poky checkout. Yocto QEMU validation must not be claimed unless `vehicle-node-image` has actually been built and booted; this workspace currently claims repository-level Yocto validation only.

## Application capabilities

- Local and embedded configuration/runtime paths
- Linux hostname, kernel, uptime, and filesystem checks
- Read-only SocketCAN/vCAN interface and CAN-tool detection
- Valid JSON status output and timestamped logs
- Signal-aware service mode and systemd integration
- Buildroot and Yocto packaging of the same source tree

The CAN check does not create interfaces or send frames. Missing CAN hardware or tools are reported safely without failing the service.

## Milestone 1: local application

```bash
./scripts/build.sh
./scripts/run_local.sh
```

Useful direct commands:

```bash
./build/vehicle-node --paths
./build/vehicle-node --status
./build/vehicle-node --health
./build/vehicle-node --check-can
./build/vehicle-node --config
./build/vehicle-node --version
```

Local output is written under `runtime/`. The user-level unit can be installed without `sudo` using `./scripts/install_service.sh`.

## Milestone 2: Buildroot + QEMU vehicle target

The Buildroot external package installs and enables `vehicle-node.service`, supplies a Buildroot-specific configuration, and creates writable runtime directories. Buildroot and its output remain outside this repository to avoid recursive local-source copying when the package synchronizes project sources.

```bash
export BUILDROOT_DIR=~/buildroot
./scripts/build_buildroot.sh
./scripts/verify_buildroot.sh
./scripts/run_buildroot_qemu.sh
```

This target was built and QEMU-verified. The service remained active, read `/etc/vehicle-node/vehicle-node.yaml`, and wrote non-empty status and log files while safely handling unavailable QEMU CAN interfaces.

## Repository Folder Name

The active project name is **Embedded Linux Vehicle Diagnostics Node**. If this repository still uses the older folder name, it can be renamed safely:

```bash
cd ~
mv audio-linux-sdk vehicle-linux-sdk
cd vehicle-linux-sdk
```

The scripts resolve the current project path dynamically, so they do not depend on the parent folder name.

## Milestone 3: Yocto vehicle layer

The repository provides `yocto/meta-vehicle-node`, containing a CMake/systemd/externalsrc application recipe and a compact `vehicle-node-image` recipe.

Repository verification requires no Poky checkout:

```bash
./scripts/verify_yocto_layer.sh
```

For an optional full build with Poky stored separately:

```bash
export POKY_DIR=~/poky
./scripts/build_yocto.sh
./scripts/run_yocto_qemu.sh
```

The external build directory defaults to `~/vehicle-node-yocto-build` and can be overridden with `VEHICLE_NODE_YOCTO_BUILD_DIR`.

Inside either embedded target, inspect the service with:

```bash
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
```

See [Buildroot integration](docs/buildroot.md), [Yocto integration](docs/yocto.md), [architecture](docs/architecture.md), and [workspace summary](docs/workspace-summary.md).

## Intentionally not included

- Real CAN message decoding
- DBC parsing
- AUTOSAR stack
- Cloud telemetry
- OTA update framework
- GUI/HMI
- Real vehicle hardware deployment

## Future improvements

- Create `vcan0` automatically in target demo mode
- Send and receive simulated CAN frames
- Add basic DBC-style signal mapping
- Add a D-Bus control interface
- Add a REST health endpoint
- Integrate watchdog notification
- Deploy to a hardware board
- Add a CI pipeline with image boot tests

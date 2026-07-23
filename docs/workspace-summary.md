# Automotive Workspace Summary

## Project direction

This project is now an Embedded Linux Vehicle Diagnostics Node, built around `vehicle-node` as a C++17 service for Linux health and SocketCAN/vCAN diagnostics, JSON status output, runtime logging, and systemd operation.

## Milestone status

| Milestone | Scope | Status |
|---|---|---|
| Automotive Milestone 1 | Local `vehicle-node` application and user systemd | Complete and verified |
| Automotive Milestone 2 | Buildroot QEMU vehicle target | Complete and QEMU-verified |
| Automotive Milestone 3 | Yocto vehicle layer | Complete at repository/layer level |

Milestone 1 and Milestone 2 are verified. Milestone 3 provides a Yocto vehicle layer verified at repository level.

## Active integration paths

```text
Local:     config/vehicle-node.yaml → runtime/{status.json,vehicle-node.log}
Buildroot: buildroot/external-vehicle-node → external Buildroot output → QEMU
Yocto:     yocto/meta-vehicle-node → external Poky build → vehicle-node-image
```

Embedded images install:

```text
/usr/bin/vehicle-node
/etc/vehicle-node/vehicle-node.yaml
/usr/lib/systemd/system/vehicle-node.service
/var/log/vehicle-node/
/var/lib/vehicle-node/
```

The application, rather than the recipes, creates and updates its log and status files.

## Repository folder name

If the local checkout still uses the older parent folder name, it can be renamed safely:

```bash
cd ~
mv audio-linux-sdk vehicle-linux-sdk
cd vehicle-linux-sdk
```

All project scripts resolve the repository path dynamically, so the workspace remains usable before or after that manual rename.

## Validation status

- Buildroot QEMU: fully verified, including boot, enabled service, restart, target config, JSON, logs, and safe missing-CAN behavior.
- Yocto layer: repository verification passed.
- Full Yocto image build: optional depending on an external Poky setup; no Yocto image or QEMU validation is claimed until that build and boot succeed.

## Scope boundary

The workspace demonstrates application design, local systemd integration, Buildroot packaging/QEMU deployment, and Yocto/BitBake layer integration. It intentionally excludes real CAN decoding, DBC parsing, AUTOSAR, vehicle control, GUI/HMI, telemetry, OTA, and real hardware deployment.

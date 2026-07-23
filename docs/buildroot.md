# Automotive Milestone 2: Buildroot and QEMU Vehicle Target

## Purpose

Milestone 2 packages the existing C++17 `vehicle-node` service into a minimal Buildroot x86_64 image and provides a QEMU target for embedded runtime validation.

Buildroot itself is not stored in this repository. The project uses `BR2_EXTERNAL` to integrate the vehicle-node package into a separate Buildroot checkout.

## External tree

```text
buildroot/external-vehicle-node/
├── Config.in
├── external.desc
├── external.mk
├── configs/vehicle_node_qemu_x86_64_defconfig
└── package/vehicle-node/
    ├── Config.in
    ├── vehicle-node.mk
    └── files/{vehicle-node.service,vehicle-node.yaml}
```

The external configuration exposes `BR2_PACKAGE_VEHICLE_NODE`. The package uses Buildroot's CMake infrastructure and the project's existing install rule to place the binary in `/usr/bin`.

## Target filesystem layout

The package installs or creates:

```text
/usr/bin/vehicle-node
/etc/vehicle-node/vehicle-node.yaml
/usr/lib/systemd/system/vehicle-node.service
/etc/systemd/system/multi-user.target.wants/vehicle-node.service
/var/log/vehicle-node/
/var/lib/vehicle-node/
```

No empty status or log file is installed. The running application writes:

```text
/var/log/vehicle-node/vehicle-node.log
/var/lib/vehicle-node/status.json
```

The wanted symlink enables automatic startup under systemd. The target configuration identifies the device as `vehicle-gateway-buildroot-01` and its platform as `buildroot-qemu`, proving that the embedded config is active.

## Build configuration

The defconfig selects x86_64, a Linux kernel, C++ support, systemd, an ext4-formatted root filesystem, a serial console, basic networking, and the vehicle package. `iproute2` and `can-utils` are enabled when available.

The root filesystem is 120 MiB. This leaves writable space for systemd state, JSON status, and logs during QEMU testing.

## External build output

Build output defaults to:

```text
~/vehicle-node-buildroot-output/buildroot-qemu-x86_64
```

It is intentionally outside the repository. A Buildroot package using a local project source synchronizes that source into its package build directory; placing output below the source would recursively copy the output into itself.

Override the location when necessary:

```bash
export VEHICLE_NODE_BUILDROOT_OUTPUT=/absolute/path/to/buildroot-qemu-x86_64
```

## Build and verify

Use a separate Buildroot checkout:

```bash
export BUILDROOT_DIR=~/buildroot
./scripts/build_buildroot.sh
./scripts/verify_buildroot.sh
```

Repository-side verification works before an image exists. After a build, the same script checks the target binary, config, service, enable symlink, kernel, and root filesystem.

## Run QEMU

```bash
./scripts/run_buildroot_qemu.sh
```

The script uses Buildroot's generated QEMU launcher if present. Otherwise it runs `qemu-system-x86_64` directly with the kernel, a writable virtio root disk, and a visible `ttyS0` console.

Inside the guest, test:

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
ls -l /var/lib/vehicle-node/
ls -l /var/log/vehicle-node/
```

## CAN/vCAN behavior in QEMU

The QEMU machine may not expose `vcan0` or `can0`. That is acceptable: `vehicle-node --check-can` reports `CAN Status: Not Available`, while software health and the systemd service remain operational. The diagnostic never creates an interface and never transmits a CAN frame.

An optional future kernel configuration could enable the virtual CAN module and create `vcan0` during target setup. That is not required for this milestone.

## What the target validates

The target validates service deployment, filesystem integration, logging, JSON status output, and safe CAN diagnostics. It does not require real CAN hardware.

## Troubleshooting

- If `BUILDROOT_DIR` is rejected, point it to a separate checkout containing the top-level Buildroot `Makefile`.
- If the custom defconfig is incompatible with the checkout, the script falls back to Buildroot's `qemu_x86_64_defconfig` and reapplies required options.
- If the service does not start, inspect `systemctl status vehicle-node` and `journalctl -u vehicle-node --no-pager`.
- If output files do not grow, check `df -h /`, directory permissions, and `vehicle-node --paths` inside the guest.
- If no CAN interface is present, treat `Not Available` as a diagnostic result rather than a service failure.

This milestone intentionally excludes real CAN traffic, DBC parsing, UDS, vehicle control, GUI components, cloud telemetry, Yocto automotive integration, and extra applications.

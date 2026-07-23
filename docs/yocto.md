# Automotive Milestone 3: Yocto Vehicle Layer

## Purpose

Milestone 3 models the existing C++17 `vehicle-node` as a reusable OpenEmbedded recipe and composes it into a compact product image. Poky is not stored in this repository. The project provides a custom Yocto layer that can be added to an external Poky build.

The verification script validates the layer without a full Yocto build. A full Yocto image build requires a separate Poky checkout and internet/cache availability.

## What Yocto adds

Buildroot directly assembles the compact QEMU target used for completed target validation. Yocto adds reusable recipe metadata, package ownership, image composition, distribution integration, and a path toward product-specific policies.

```text
yocto/meta-vehicle-node/
├── conf/layer.conf
├── recipes-apps/vehicle-node/
│   ├── vehicle-node_1.0.bb
│   └── files/{vehicle-node.service,vehicle-node.yaml}
└── recipes-core/images/vehicle-node-image.bb
```

## Application recipe

`vehicle-node_1.0.bb` inherits `cmake`, `systemd`, and `externalsrc`. Since the layer is at `PROJECT_ROOT/yocto/meta-vehicle-node`, the recipe resolves the project root by traversing two parent directories from `LAYERDIR`. This development-oriented arrangement builds the existing source tree; a production recipe would normally fetch an immutable revision.

Recipe-owned service and configuration assets are unpacked into `WORKDIR`. CMake installs the executable, while the recipe installs the target configuration and systemd unit, creates the runtime directories, packages those paths, and enables `vehicle-node.service`.

`iproute2` is included as a runtime dependency. `can-utils` is intentionally not mandatory: the diagnostics remain healthy when `candump`, `cansend`, or CAN interfaces are absent.

## Image recipe and systemd

`vehicle-node-image.bb` extends `core-image-minimal`, installs `vehicle-node`, and enables Dropbear. It selects systemd for this standalone demonstration. The build script also adds the systemd and qemux86-64 settings idempotently to the external build's `conf/local.conf`.

## Target filesystem layout

```text
/usr/bin/vehicle-node
/etc/vehicle-node/vehicle-node.yaml
/usr/lib/systemd/system/vehicle-node.service
/var/log/vehicle-node/
/var/lib/vehicle-node/
```

The application creates `/var/log/vehicle-node/vehicle-node.log` and `/var/lib/vehicle-node/status.json` at runtime. The Yocto configuration identifies `vehicle-gateway-yocto-01` in `yocto-qemu` mode.

## Verify, build, and run

Repository-only verification:

```bash
./scripts/verify_yocto_layer.sh
```

Optional image build and QEMU run:

```bash
export POKY_DIR=~/poky
./scripts/build_yocto.sh
./scripts/run_yocto_qemu.sh
```

Poky and the default output directory `~/vehicle-node-yocto-build` remain outside the repository. Override the output with `VEHICLE_NODE_YOCTO_BUILD_DIR` if needed.

Inside QEMU:

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

## Validation status

Buildroot QEMU target validation is fully completed. Yocto layer verification can be completed without Poky. Full Yocto QEMU validation should only be claimed after building and booting `vehicle-node-image`; it is not claimed by repository verification alone.

## Troubleshooting

- If `POKY_DIR` is rejected, point it to an external checkout containing `oe-init-build-env`.
- If the layer series is rejected, confirm the Poky release and update `LAYERSERIES_COMPAT_vehicle-node` only after checking compatibility.
- If source lookup fails, keep the layer at `PROJECT_ROOT/yocto/meta-vehicle-node` so the two-level `EXTERNALSRC` traversal remains valid.
- If service/config assets are missing, confirm they reached `WORKDIR` and inspect `do_install`.
- If systemd is unavailable, inspect the external build's `conf/local.conf` and distribution features.
- If the image is missing, run `./scripts/build_yocto.sh` before the QEMU launcher.

## Intentionally not included

This milestone excludes Poky source, in-repository Yocto output, real CAN decoding, DBC parsing, AUTOSAR, GUI/HMI, telemetry, OTA, and vehicle hardware deployment.

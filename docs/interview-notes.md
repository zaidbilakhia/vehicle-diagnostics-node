# Automotive Embedded Linux Interview Notes

## Project summary

This is a compact Embedded Linux Vehicle Diagnostics Node built around one portable C++17 service. The same application supports local Linux development, a fully tested Buildroot QEMU image, and a repository-verified Yocto integration layer.

## Why I built it

I wanted to understand the full application-to-image path used in automotive embedded products: Linux system programming, service supervision, target configuration, cross-compilation metadata, filesystem integration, image composition, and honest validation boundaries.

## Milestone 1 explanation

I built `vehicle-node` with dependency-free configuration parsing, Linux health inspection, read-only SocketCAN/vCAN availability checks, valid JSON status output, timestamped logging, local/embedded path selection, signal-aware service behavior, and local systemd support.

## Milestone 2 explanation

I created a Buildroot external package and x86_64 image, kept build output outside the repository to prevent recursive local-source copying, and booted the target in QEMU. I verified automatic systemd startup, target configuration, service restart, non-empty status and log files, and healthy behavior without a QEMU CAN interface.

## Milestone 3 explanation

I refactored the legacy metadata into `meta-vehicle-node`. Its BitBake recipe builds the existing CMake project through `externalsrc`, installs target assets, packages runtime directories, and enables the systemd service. A custom minimal image recipe includes the package. Repository verification is complete; a full image build remains dependent on an external Poky checkout.

## Tools used

C++17, CMake, Linux `/proc` and `/sys`, SocketCAN/vCAN diagnostics, systemd, Bash, Buildroot, QEMU, Yocto/OpenEmbedded, BitBake metadata, and `externalsrc`.

## What problems I solved

- Shared local and embedded path behavior without separate binaries
- Safe JSON and log file creation at runtime
- Missing CAN interfaces and tools without service failure
- Signal-aware long-running service shutdown
- Buildroot local-package recursion caused by in-repository output
- Target filesystem capacity and writable runtime directories
- Automatic service enablement in two embedded build systems
- Target-specific local, Buildroot, and Yocto configurations

## What I learned

An embedded application is only one layer of a product. Configuration ownership, writable state, init integration, cross-compilation metadata, dependency policy, filesystem sizing, and validation evidence determine whether it behaves correctly on target.

## How this relates to automotive embedded Linux

Automotive Linux systems need supervised services, clear target identity, health reporting, CAN interface visibility, deterministic packaging, and integration with product distributions. This project demonstrates those foundations without pretending to implement safety-critical control, AUTOSAR, or complete vehicle diagnostics protocols.

## What I would improve next

I would add automatic demo-mode `vcan0` setup, simulated CAN frame exchange, basic DBC-style signal mapping, D-Bus control, watchdog notification, a small health endpoint, CI image boot tests, and deployment to a representative hardware board.

## CV bullet

Developed a C++17 automotive Embedded Linux vehicle diagnostics node with systemd startup, SocketCAN/vCAN checks, configuration management, health diagnostics, JSON status output, Buildroot QEMU image validation, and Yocto layer/BitBake recipe integration.

## Short interview answer

I built a compact automotive Embedded Linux vehicle diagnostics node in C++17. It runs as a systemd service, reads configuration, checks Linux health and SocketCAN/vCAN availability, writes JSON status output and runtime logs, and supports both local and embedded filesystem paths. I validated it locally, packaged it into a Buildroot QEMU image, and verified the service inside the embedded target. I also created a Yocto layer and BitBake recipe structure so the same service can be integrated into a Yocto-based product image.

## Long interview answer

I started with a deliberately small C++17 Linux service so every part remained explainable. It loads a constrained YAML-style configuration, reads hostname, kernel, uptime, and filesystem state, checks configured SocketCAN/vCAN interfaces and common CAN tools without sending traffic, and writes operational logs plus machine-readable JSON health state. Path selection lets the same executable use project-local files during development and standard `/etc` and `/var` paths on embedded targets.

Next, I packaged the source with a Buildroot external tree and booted it under QEMU with systemd. Target work exposed practical integration problems: keeping a local-package output directory inside the source tree caused recursive copying, and runtime data required deliberate filesystem capacity. The corrected image builds outside the repository, starts the service automatically, uses its Buildroot-specific identity, and continues operating safely when QEMU has no CAN interface.

For Yocto, I modeled the application as an OpenEmbedded recipe and custom image layer. The recipe uses CMake, systemd packaging, a Yocto-specific configuration, and local `externalsrc` development integration. The layer is repository-verified without vendoring Poky or generating build output in the project. A full Yocto image build and QEMU boot require an external Poky environment and are not claimed as completed here.

## Honest validation status

Buildroot QEMU validation is fully tested. The Yocto layer is implemented and repository-verified; full Yocto image validation depends on a local Poky setup.

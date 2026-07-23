# Automotive Milestone 1 Architecture

`vehicle-node` is a small automotive Embedded Linux diagnostics service. Its modules separate configuration, platform inspection, CAN availability checks, runtime output, and orchestration.

```text
config/vehicle-node.yaml
        ↓
vehicle-node C++ service
        ↓
system health + SocketCAN/vCAN check
        ↓
runtime/vehicle-node.log
runtime/status.json
        ↓
systemd service supervision
```

## Components

- **Config:** Parses the supported `key: value` format, ignores blank and comment lines, trims whitespace, and retains defaults for missing or invalid values.
- **Logger:** Creates the selected log directory and appends flushed, timestamped service messages.
- **SystemInfo:** Reads hostname, kernel release, and uptime through standard Linux interfaces.
- **CanCheck:** Checks the primary and fallback interfaces below `/sys/class/net` and reports availability of `ip`, `candump`, and `cansend` on `PATH`. It neither creates interfaces nor sends frames.
- **VehicleNode:** Implements CLI commands, health checks, JSON status writing, path reporting, and the signal-aware service cycle.
- **systemd integration:** Supplies a target-oriented unit plus a script that adapts it for local user-service testing.

## Paths

Local development uses:

```text
config/vehicle-node.yaml
runtime/vehicle-node.log
runtime/status.json
```

An embedded installation uses:

```text
/etc/vehicle-node/vehicle-node.yaml
/var/log/vehicle-node/vehicle-node.log
/var/lib/vehicle-node/status.json
```

`VEHICLE_NODE_CONFIG` overrides the configuration path. `VEHICLE_NODE_RUNTIME_DIR` places `vehicle-node.log` and `status.json` together in a selected directory.

## Health and CAN availability

Software health and CAN availability are separate results. Health describes whether configuration, runtime output, system information, and the diagnostic operation work. CAN is available when either the configured primary or fallback interface exists. Missing hardware or optional CAN utilities does not make the service crash and does not automatically make software health fail.

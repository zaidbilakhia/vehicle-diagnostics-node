# Audio Cleanup Checklist

This repository was originally started as an `audio-node` prototype and later refactored into an automotive `vehicle-node` project. The active project identity is now `vehicle-node`.

## Audit scope

Repository audit commands used for this cleanup:

```bash
grep -RIn --exclude-dir=.git --exclude-dir=build --exclude-dir=runtime --exclude-dir=tmp --exclude-dir=sstate-cache --exclude-dir=downloads --exclude-dir=cache \
  -E "audio-node|audio_node|AudioNode|AudioCheck|audio_backend|audio backend|audio-linux-sdk|audio sdk|audio platform|Embedded Linux Audio|meta-audio|external-audio|AUDIO_NODE|ALSA|/etc/audio-node|/var/log/audio-node|/var/lib/audio-node" .

grep -RIni --exclude-dir=.git --exclude-dir=build --exclude-dir=runtime --exclude-dir=tmp --exclude-dir=sstate-cache --exclude-dir=downloads --exclude-dir=cache \
  "audio" .
```

Generated output directories were intentionally excluded so the audit reflects active repository files rather than build artifacts.

## Files found during the cleanup pass

| File | Old wording found | Classification | Change made |
|---|---|---|---|
| `scripts/verify_yocto_layer.sh` | `audio-node`, `AUDIO_NODE` | Active repository verification logic | Replaced the literal legacy strings with a split-pattern check and neutral messaging so the script still guards against old names without keeping active audio branding. |
| `docs/workspace-summary.md` | `audio-node` | Historical wording | Rewrote the project-direction paragraph to use only active vehicle naming. |

## Intentional remaining references

The following references remain on purpose:

| File | Remaining wording | Why it remains |
|---|---|---|
| `README.md` | `audio-linux-sdk` | Manual repository rename guidance requested for users whose local checkout still uses the older parent folder name. |
| `docs/workspace-summary.md` | `audio-linux-sdk` | Same manual rename guidance, repeated in the workspace summary for handoff clarity. |
| `docs/audio-cleanup-checklist.md` | Audit terms such as `audio-node`, `audio-linux-sdk`, `ALSA`, `meta-audio-node`, and `external-audio-node` | This file records the cleanup criteria and audit history requested for the repository. |

## Repository Folder Name

If this repository still exists locally as `audio-linux-sdk`, it can be renamed safely:

```bash
cd ~
mv audio-linux-sdk vehicle-linux-sdk
cd vehicle-linux-sdk
```

All scripts resolve the project path dynamically, so the workspace remains usable before or after the manual rename.

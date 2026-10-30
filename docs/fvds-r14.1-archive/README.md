# FVDS R14.1 experiment snapshot

Historical snapshot of the closed FVDS research line (R12–R14.1). **Nothing here is wired into the build or into the released NVL1/NVA1 playback path.** Conclusion of the line: [`../FVDS-RESEARCH-ARCHIVE.md`](../FVDS-RESEARCH-ARCHIVE.md).

This directory exists so the final experiment sources stay readable after the research branches are deleted. The authoritative recovery basis is still the private frozen archive (`FVDS_Research_Final_20261030`) with its manifest and Git bundle.

## What is here

| Path | Meaning |
|---|---|
| `r14.1-staged.patch` | The complete R14.1 working-tree change, exported from the private worktree without alteration. Base commit `ba8fd9cd3775a68723e1a200e37c9281c1ab01de`. 23 files, 1151 added lines. |
| `probe/good_geom.c` | Final GPU probe. 48 quads of 32×32, six 128×64 sub-textures, single flush per frame. This is the configuration that rendered a complete frame. |
| `probe/ramp48_v2.c` | Earlier 48-quad variant with the coordinate-ramp texture, kept because the mapping was calibrated against it. |
| `probe/config_ramp48.h`, `probe/b.sh`, `probe/build.sh` | Build configuration and helper scripts for the probe. |
| `probe/refplanes.{c,h}` | One decoded 256×192 plane (three 8-bit planes, 147 456 bytes of payload as a text table) so the probe is self-contained. |
| `evidence/*.png` | Three captures: calibrated gradient mapping, the reference plane at 32 px blocks, and one of the intermittent empty frames. |

## Provenance

Everything was copied byte-for-byte out of the private frozen archive; the SHA-256 of each file is in `SHA256SUMS.txt`. Nothing was edited for publication, so the source comments still carry R14.1's own (partly outdated) assumptions — notably the header of `good_geom.c` claims the hardware filter degenerates to nearest neighbour. It does not: the GE applies a 2×2 filter, which halves vertical resolution on 8 px stripes and is why the plane capture is a blur rather than a pixel-exact match.

`probe/refplanes.c` is a decoded frame derived from the private test asset. It contains no VLC decode tables.

## Publication screening

- The patch was screened with the repository's own checker, run on a tree restored from the private bundle at the patch's base commit:

  ```
  public patch OK: 23 file(s), 1151 added line(s), no table payload
  ```

  `tools/public-patch/check-public-patch.py` and `allow.txt` arrive with the patch itself, so the check can be repeated after applying it. It needs the private table tree; without it the checker fails closed.
- The probe sources include only `<nds.h>`, `<fat.h>`, `<stdint.h>`, `<stdio.h>`, `<string.h>` and their two local headers. No private table header, no base64 payload, no compressed blob.

## Deliberately not published

- Private VLC decode tables (`fvds-private/tables`, `fvds-private/held`).
- The complete archive ZIP and the 26 MB all-ref history bundle — both carry the private tables.
- Game assets / ROMs and the FVDS stream itself.
- The 115-test run (`101 passed, 14 skipped, 0 failed`) needs the private tables configured, so it cannot be reproduced from this directory alone.

## Reproducing the probe

```sh
# apply the change on its base commit, then replace and build the probe
git apply --binary r14.1-staged.patch
cp probe/good_geom.c cpymo-backends/nds/tools/fvds-gpu/main.c
cp probe/config_ramp48.h cpymo-backends/nds/tools/fvds-gpu/config.h
cp probe/refplanes.c probe/refplanes.h cpymo-backends/nds/tools/fvds-gpu/
```

`SCEN` selects the motion scenario, `RAMP` swaps the plane for a coordinate ramp, `PRODUCT 0/1` selects CPU-side reference or GPU path.

Single screenshots hit an empty frame about one time in three, so always take several and keep the one with the highest variance.

## Known limits (short form)

- Blocks above 64 quads per frame truncate in melonDS; 48 works. Not a proven hardware limit.
- Intermittent empty 3D frames remain.
- No real hardware timing, no continuous playback, no audio/video sync.
- The texture upload, not the geometry, dominates the measured simulator cost.

# FVDS video research (R12–R14.1) — archived

**Status:** experimental research concluded on 2026-10-30. **Not a supported CPyMO video backend.** The released NDS 1.2.0 NVL1/NVA1 playback path remains unchanged.

## Why this work was explored

FVDS offered substantial file-size reduction on one same-source test asset: video data approximately 23.85 MB → 5.14 MB (−78.5%), and video plus audio approximately 30.86 MB → 12.94 MB (−58.1%). This is **not** a quality-matched or real-time-performance certification.

The project investigated an independent decoder and Nintendo DS 3D-assisted reconstruction after the original CPU-only experiments failed to establish sustained 12 FPS playback.

## Results and remaining limitations

R14.1 reached reproducible geometry/texture mapping in limited synthetic tests. A 48-quad configuration could display a calibrated pattern in melonDS. The local research handoff reports **115 host tests executed: 101 passed, 14 skipped, 0 failed** (the private test-table environment must be configured). These tests are not proof of actual hardware playback.

At closure, the following were **not validated**:

- Correct reconstruction of all real FVDS I/P frames against an independent pixel reference.
- More than 64 quads per frame in the current melonDS submission path; the reported truncation is **not a proven Nintendo DS hardware vertex limit**.
- Consistent rendering without intermittent empty 3D frames.
- Real NTR/DSi hardware end-to-end throughput, including upload, geometry, sync, and decode.
- Continuous video playback and actual audio/video synchronization.

Simulator-only upload timing and CPU-copy comparisons must not be interpreted as a speedup of the complete decoder.

## Closure decision

Further engineering was estimated to exceed the project's previously agreed **one-week remaining-work limit**. Therefore this experimental route is **paused/archived**, not declared technically impossible and not merged into the supported playback path. No new research rounds are planned.

The final R14.1 local frozen evidence, staged source changes, and Git bundle are catalogued in the private project archive `FVDS_Research_Final_20261030`. **This branch does not, by itself, attest that all local R14.1 files have been published here.** The final locally preserved archive and manifest are the authoritative recovery basis until separately reconciled with GitHub.

## Reuse and publication boundaries

- Keep the completed CPyMO NDS 1.2.0 release, its tags, and the current NVL1/NVA1 backend intact.
- Preserve the experimental research branches and local full-history bundle until the final archive and branch deltas are verified.
- Do **not** commit private `fvds-private` tables, game assets, or code/data of unclear redistribution status to this public repository.
- Any future reuse requires a fresh technical and licensing assessment; archived results are not production integration instructions.

This document records the conclusion of the research. It does not announce an FVDS release or claim verified playback on DS/DSi hardware.

## Final private handoff verification (2026-10-30)

The complete R14.1 freeze package was received and independently checked before private archival:

- Source ZIP SHA-256: `60c292c51a8c32b068d2102206dae7565087edf4562ce42ee29e1304e18ff2d8`.
- ZIP content: 83 files; all 81 files listed in its primary `SHA256SUMS.txt` matched. Both checksum-list files are deliberately excluded from the primary list.
- Git bundle: complete-history verification, independent clone, and object connectivity verification succeeded.
- The preserved staged R14.1 diff applies cleanly to its recorded base commit `ba8fd9cd3775a68723e1a200e37c9281c1ab01de`.
- The archived public-patch screening procedure reported 23 allowed paths and no private table payload in that diff. This check does **not** certify all archived material as redistributable.
- The 115-test result above is the **local handoff's reported outcome**, not a new CI run for this branch.

A private archive ZIP was rewrapped solely to correct ZIP filename encoding flags; individual extracted file bytes were verified unchanged. Its SHA-256 is `d56f4ed211bfd12584485fd27babc13ae042d0ebce38bf824269e43eeb9710aa`.

**Do not commit or attach either complete ZIP to this public repository.** Both contain the isolated `private-tables/` test assets and full research history. The complete archive is retained in the user's private FVDS project Library; this public branch records the conclusion only. Any future publication of experiment source requires a separate provenance review.

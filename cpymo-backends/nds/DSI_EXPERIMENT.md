# CPyMO for NDS/DSi · Experimental development record

This document records verified facts separately from hypotheses. It applies only to branch `codex/nds-dsi-runtime-experiment`; the published `nds-v1.1.0` release remains unchanged.

## Hardware confirmation (2026-10-21)

- User's original Nintendo DSi, DSpico USB-C, AKMenu-Next v3.0.1.
- Homebrew Loader remained **AKMenu-Next (default)**; **Pico-Loader was not selected**.
- CPyMO R1 reported `TWL H:02100D00-02FCC000`.
- The `TWL` label is based on libnds `isDSiMode()`, aliased to Calico `systemIsTwlMode`.
- `H:` reports `getHeapStart()` and `getHeapLimit()`. The window spans `0xECB300` bytes (15,512,320 bytes, approximately 14.79 MiB). This is *not* a measured count of free/allocatable bytes.
- This confirms a TWL runtime under the default homebrew launcher, but does not directly measure clock speed, sustained throughput, stable large allocations, video frame rate or audio synchronization.
- As separately reported, the NDSi's internal SD card is currently necessary for successful AKMenu-Next boot in this setup. Do not conflate this menu compatibility issue with CPyMO's storage requirements.

## Experimental changes

- Replace hardcoded `DS / DS Lite · 4 MiB` splash/about text with runtime NTR/TWL indication and heap address range.
- Keep a separate experimental DSi ROM output and CI artifact without modifying the published 1.1.0 release.
- Retain the original 384 KiB recent-sprite cache in NTR. Raise its **maximum budget** to 2 MiB only when `isDSiMode()` is true and the runtime heap window spans at least 8 MiB; the cache continues to hold at most six recent images. This change does not rewrite the heap allocator or force CPU frequency. Hardware performance impact is not yet measured.

## Next functional milestone: video

The 1.1.0 source already includes an experimental NVL1/NVA1 RGB555, 256×192 video decoder with 22050 Hz mono streaming audio, but the released converter intentionally discards movie files and retains their references in its audit. Do not represent this as supported video playback.

The next isolated experiment should:

1. Add an **opt-in**, backward-compatible FFmpeg conversion path for short test video clips; default conversion and source media stay unchanged.
2. Match exactly the runtime `nds_cache_path` hashing/lookup and NVL1/NVA1 packet framing, frame checksum and audio offsets.
3. Verify a short synthesized clip end-to-end before trying full OP movies; compare 8/12/15/24/30 fps candidates and byte rates without promising a target.
4. Compare CPU cost, maximum draw+decode time, underflows, memory peak, source-card I/O and audiovisual skew on real DSi, then test normal DS/R4 regression.
5. Only consider enabling the feature by default after hardware testing; preserve the normal DS-compatible build.

## Additional backlog

- Explicitly mount and distinguish `fat:` (flashcart storage) versus `sd:` (console SD) before offering dual-card CPyMO library selection. Browsing both in AKMenu-Next does **not** imply CPyMO automatically scans both.
- Investigate the separate AKMenu-Next no-console-SD boot problem after CPyMO functionality is established.

No upstream PR or release should be made solely on the strength of this experiment.

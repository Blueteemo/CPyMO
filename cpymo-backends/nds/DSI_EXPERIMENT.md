# CPyMO for NDS/DSi · Experimental development record

This document records verified facts separately from hypotheses. It covers follow-on experimental branches derived from `codex/nds-dsi-runtime-experiment`, including `codex/nds-dsi-dual-card-library` and `codex/nds-dsi-library-scrollbar`. The published `nds-v1.1.0` release remains unchanged.

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

## Dual-card hardware acceptance and library UX (2026-10-21)

- R3 builds on the TWL-capable loader and scans the current storage plus the other device's `sd:/pymo/games` or `fat:/pymo/games` and card root, using *absolute per-device* game paths. Game configs, scripts, caches, and saves remain together on their own card.
- Actual user test: original DSi with DSpico/AKMenu-Next and separate **128 MB console SD**. The converted original PyMO synthetic game `dualcard_echo_30m` occupied **31,035,615 bytes** (29.60 MiB); game data was generated through the **1.1.0 official NDS converter**, 12 scenes, 4 characters, 8 BGM tracks, 0 missing references or unknown commands. The user reports R3's cross-card game test **passed**. This verifies the basic dual-card discovery/play route; do not claim all chapter, save/load, or full-game scenarios individually tested without additional evidence.
- Follow-on R4 library polish: show a proportional right-edge scrollbar **only with more than four games**; replace four-item page jumps with a four-row rolling viewport that follows D-pad navigation, works with touch selection, and also handles library refresh, selection restore, and wraparound. With five games, selecting the fifth displays games **2–5**, not the fifth alone.
- Normal DS/R4 retains its single-card path and the 384 KiB image-cache budget; TWL with a suitable heap window may use a bounded 2 MiB recent-image budget. No shared CPyMO script, save or interpreter formats are modified.
- R4 is code/CI validated; the newly added scrollbar interaction still needs a user's 5-game real-device confirmation.

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

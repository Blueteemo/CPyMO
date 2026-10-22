# CPyMO for NDS 1.2 — Public source / ROM release gate

**Status (2026-10-23): mandatory review hold on the received FVDS *full-video*
decoder, not on the separately developed NDS UI, container reader or other
reviewed CPyMO work.** This is a project engineering and provenance checklist,
not a legal opinion or a claim that any abstract codec algorithm is protected.

## Why the decoder cannot simply be pushed

- This repository carries **AGPL-3.0**. The public
  [Gericom/FastVideoDSPlayer](https://github.com/Gericom/FastVideoDSPlayer)
  and [Gericom/FastVideoDSEncoder](https://github.com/Gericom/FastVideoDSEncoder)
  repositories currently expose **no repository license**. Upstream
  [license request #15](https://github.com/Gericom/FastVideoDSPlayer/issues/15)
  remained open when checked on 2026-10-23.
- The local DeepSeek handoff `gen_vlc_tables.py` explicitly opens
  `Gericom.FastVideoDS/VlcTables.cs` and reads `TableA`, `TableB` and
  `TableARefLinear` to generate the ~84 KiB `fvds-vlc-tables.h`.
  This is **vendor-table extraction**, not solely independent decoding
  of a published binary format. Calling the rest of the decoder
  "written from scratch" does not grant redistribution rights for the
  generated tables or establish clean provenance for other copied/derived
  implementation details.
- The local M1 bridge and received decoder have **not** been merged or
  statically linked into public ROM builds. Host tests and private research
  cannot be cited as permission to distribute a bundled binary.

## Release-safe implementation tracks

1. **Preferred: explicit upstream permission.** Seek a published suitable
   licence for the necessary encoder/player source and data tables, or a
   written grant from the relevant copyright holders that expressly permits
   modification, source redistribution and distribution **within our
   AGPL-3.0 CPyMO combined work**. Verify the precise coverage of
   `VlcTables.cs`, transforms, predictor and motion compensation, and
   compatibility of any conditions. Record evidence, authors and attribution.
   Do not assume an issue comment with unclear scope is sufficient.
2. **Independent replacement when permission is unavailable.** Treat the
   existing vendor-derived M1 as a *quarantined research reference*, not
   publishable source. Work from observable FVDS bitstreams, documented
   field syntax and independently authored test vectors/specifications.
   Generate or derive the required decoding constants without reading or
   transcribing the proprietary vendor tables. Because the previous worker
   already inspected those tables, do **not** automatically label a rewrite
   by the same worker "clean-room". Maintain separate provenance and
   independent review of the replacement before distribution.
3. **Nonblocking feature fallback.** If the codec rights remain unresolved
   near the final release, the ordinary 1.2 NDS/DSi source and ROM must
   remain buildable and publishable with existing, independently developed
   NVL1/NVA1 video support and all accepted dual-screen features. Do not
   publish a video-capable binary containing the quarantined code, nor
   claim that this fallback is FVDS playback.

## Mandatory acceptance evidence before public FVDS enablement

- [ ] Upstream permission covering every used file/table **or** an
      independently verifiable implementation/provenance review.
- [ ] SPDX/license inventory, source links, attribution and licence notices
      for any third-party material; AGPL combination compatibility checked.
- [ ] A public, reproducible generator / decoder source (not a hidden
      `M1.zip` import); no user-owned MP4, FVDS or extracted film frames
      in Git history or release artifacts.
- [ ] Independent real-video comparison, malformed data tests,
      q-value handling, 4 MiB peak memory and DS/DSi playback timing.
- [ ] NTR/TWL ROM and converter CI, real hardware B skip, EOF/error,
      BGM/audio restoration and user acceptance.
- [ ] Only after satisfying the above, explicitly update or remove the
      quarantined-source CI hold in `test_nds_fvds_release_gate.py` as a
      documented change with reviewer approval. Deleting the test alone is
      not evidence of permission.

**Safe now:** retain a private host prototype and continue publicly
developing format-neutral 12 fps scheduling, ARM7 stereo queue, VRAM
ownership and generic `#movie` cleanup. Neither the existing stable R7
nor the public CPyMO 1.1.0 release is changed by this policy.

# CPyMO for NDS 1.2.0 — Paired candidate artifact manifest

**Status: RC review only, not published.** Built by [GitHub Actions run 40494654625](https://github.com/Blueteemo/CPyMO/actions/runs/40494654625) from source commit `213d756a5467d77b7da3a52b142337f2946e7bec`. All five jobs succeeded. Files below were downloaded from that run, unwrapped from GitHub Actions' artifact ZIP and independently checked for archive integrity and paired ROM hashes.

| Artifact | Byte size | SHA-256 |
|---|---:|---|
| `cpymo-nds.nds` (the same ROM stored locally as `cpymo-nds_1.2.0_approved_universal_RC.nds`) | 924,672 | `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31` |
| Windows Starter RC (`CPyMO-for-NDS-1.2.0-Starter-Windows-RC.zip`) | 57,686,077 | `9859815c8fa5174e637da418236b24e84e0fe6590992439d7414056c9d59a055` |
| macOS LGPL Starter RC (`CPyMO-for-NDS-1.2.0-Starter-macOS-LGPL-RC.zip`) | 54,992,313 | `d99ca80e64ecdb098cb4e116f4eea7625109dbb9869c597e70dd6f4176168546` |
| CPyMO corresponding source RC (`CPyMO-for-NDS-1.2.0-Sources-RC.zip`) | 21,775,598 | `ee8e067863cf2007a21df97204313946fafc96fb57a8dfee47380da7cea010c3` |

Verification:
- Both Windows and macOS installer archives contain two readable NDS ROM copies, each exactly 924,672 bytes with **identical** SHA-256 matching the above universal ROM.
- The universal ROM contains one approved embedded 02-B background: RGB555 pixel SHA-256 `92d91514ac12ccab6037a86a71e0c4cf1047917d15de59b91bcc8e4bb4d7d1b2`. No placeholder art may ship.
- The Windows and macOS archives both carry **the identical official** `FFmpeg-Corresponding-Source/ffmpeg-7.1.5.tar.xz` (11,050,340 bytes; SHA-256 `de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f`), captured build configuration, license output, binary and source checksums. FFmpeg is built with `--disable-gpl --disable-nonfree`. Windows frozen conversion tested an original synthetic MP4 into NVL1; the macOS frozen worker tested default NVL1, box fallback and source immutability.
- The macOS app is **not signed with Apple Developer ID or notarized**. `macos-15-intel` CI success is *not* a Finder GUI or Apple Silicon test. A separate user-local DeepSeek test is pending.
- macOS FFmpeg may be ad-hoc codesigned by PyInstaller, so the executable in `.app/Contents/Frameworks` need not hash identically to the source-built input. Version 1.2's macOS packager now records both hashes separately for subsequent RC builds.

**Important non-final constraints:** The run predates a later packager metadata-only adjustment (`8b17e7f`). This package set therefore remains an immutable RC snapshot, not the final release. Regenerate all public files and final SHA256SUMS from one frozen commit after local Mac acceptance; do not silently rename these RC hashes to final Release assets. Keep the prior macOS prebuilt `--enable-nonfree` candidate entirely out of public distribution.

Release order: resolve owner-local Mac review; final source/license/notices audit; freeze; rebuild all, compare checksums; publish GitHub Release `nds-v1.2.0`; then promote the second PyMO logo and bespoke NDS README to repository root; lastly revise upstream PR #60 body using real Release link.

## Latest bilingual ARM64 / Windows RC cross-platform audit (2026-10-27)

All five jobs succeeded on [GitHub Actions run 40505159429](https://github.com/Blueteemo/CPyMO/actions/runs/40505159429), source commit `83c0b6e9bd9118780c962508f1db50d6e3a2d041`. This is the **paired ARM64-only macOS + bilingual converter** candidate offered for local DeepSeek Finder acceptance.

The downloaded GitHub artifact ZIPs each contain one nested distribution ZIP. Numbers below are for **nested distribution files**, not GitHub transport wrappers:

| Candidate distribution | Exact bytes | SHA-256 |
|---|---:|---|
| `CPyMO-for-NDS-1.2.0-Starter-macOS-arm64-RC.zip` | 53,173,599 | `84af5c2c78f45527c355d8febe104e9a3e101527c63773a48f27ba4f14f224c1` |
| `CPyMO-for-NDS-1.2.0-Starter-Windows-RC.zip` | 57,696,706 | `04d1676f07f61cd74c3cf5be3996d692002ae186a00b4a66b045120dd73c9881` |
| `CPyMO-for-NDS-1.2.0-Sources-RC.zip` | 21,789,169 | `c62afd9eca8e89049f7d11d927429cde15600f63c1b282b9d30172d517816d34` |
| `cpymo-nds.nds` (ROM transport) | 924,672 | `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31` |

Additional independent archive audit:
- All nested ZIPs passed integrity tests; Windows and ARM64 Mac both contain matching installed and bundled ROM **regular files** with the approved SHA-256.
- In the macOS `.app`, `Contents/Frameworks/cpymo-nds.nds` is a **26-byte symbolic link** to `../Resources/cpymo-nds.nds`, not an extra ROM. Hashing that link's literal target path falsely reports a mismatch. Likewise `Contents/Resources/ffmpeg` links to `../Frameworks/ffmpeg`.
- Both desktop RCs contain the **same original** FFmpeg 7.1.5 source archive: 11,050,340 bytes, SHA-256 `de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f`.
- macOS `VERSION.json` says `macos_architecture: arm64`, `rosetta_required: false`, and separately records FFmpeg input and post-bundle hashes. The owner-local Finder/Gatekeeper experience is *not* yet attested.
- The matching CPyMO source archive includes `ui_i18n.py`, `tests/test_ui_i18n.py`, and the proposed NDS-specific README. Intel Mac native binaries are excluded by the ARM64-only release policy.

**The RC is not a published release.** The source snapshot predates later QA/documentation-only commits; final public asset hashes require a frozen corresponding-source commit and cross-package audit. Upstream PR #60 must only be revised *after* the actual Release.

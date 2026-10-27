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
- The Windows and macOS archives both carry `FFmpeg-Corresponding-Source/ffmpeg-7.1.5.tar.xz`, captured build configuration, license output, binary and source checksums. FFmpeg is built with `--disable-gpl --disable-nonfree`. Windows frozen conversion tested an original synthetic MP4 into NVL1; the macOS frozen worker tested default NVL1, box fallback and source immutability.
- The macOS app is **not signed with Apple Developer ID or notarized**. `macos-15-intel` CI success is *not* a Finder GUI or Apple Silicon test. A separate user-local DeepSeek test is pending.
- macOS FFmpeg may be ad-hoc codesigned by PyInstaller, so the executable in `.app/Contents/Frameworks` need not hash identically to the source-built input. Version 1.2's macOS packager now records both hashes separately for subsequent RC builds.

**Important non-final constraints:** The run predates a later packager metadata-only adjustment (`8b17e7f`). This package set therefore remains an immutable RC snapshot, not the final release. Regenerate all public files and final SHA256SUMS from one frozen commit after local Mac acceptance; do not silently rename these RC hashes to final Release assets. Keep the prior macOS prebuilt `--enable-nonfree` candidate entirely out of public distribution.

Release order: resolve owner-local Mac review; final source/license/notices audit; freeze; rebuild all, compare checksums; publish GitHub Release `nds-v1.2.0`; then promote the second PyMO logo and bespoke NDS README to repository root; lastly revise upstream PR #60 body using real Release link.

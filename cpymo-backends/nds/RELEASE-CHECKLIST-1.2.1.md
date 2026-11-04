# CPyMO for NDS 1.2.1 — final release gates

Status: release candidate, **not yet a public GitHub Release**.

## Approved product baseline

- [x] v1.2.1 final ROM based on test4, with Default RGB555 rail (15,26,30).
- [x] 16 console IPL colors, cached Auto palette, theme-matched readable text, 2-pixel selection rail.
- [x] Title auxiliary guide reset on in-game Restart; START launcher action; RGB555 Alpha rounding.
- [x] Conservative +1.5 dB audio gain with lookahead peak guard; no game-volume changes.
- [x] Final image design selected: NDS full square, Windows/macOS companion converter rounded, from same user-supplied illustration.
- [x] NDS 32x32/15-color banner and compiled fallback icon resources updated together, with matching source-level regression tests.
- [ ] Desktop converter source and deterministic build generator changed from the previous outline-cleanup artwork to the new round-mask illustration.
- [ ] New desktop icon source licensing/attribution and final Windows/macOS binary embedding audit completed.
- [x] 1.2.1 metadata, release notes draft, and NDS documentation updated on the candidate branch.

## Immutable first-pass paired release candidate

- First paired CI: https://github.com/Blueteemo/CPyMO/actions/runs/41901139830
- CI snapshot: `b8b07a805edc3a6d6bf5504d6c8720e92ed547e9` (before the later documentation-only commit).
- Verified universal ROM: 935,936 bytes, SHA-256 `9046c0440109f6c196ac7af0106625bc5407fbf5128e35beab07d8cff38b8de2`.
- [x] NTR/TWL combined header, approved 02-B background, standalone ROM validation.
- [x] Earlier converter regression and prior pinned icon generation (historical; repeat after the new desktop icon integration).
- [x] Source RC ZIP, original icon and third-party attribution included.
- [x] Windows frozen build + synthetic video conversion, exact ROM pairing, FFmpeg 7.1.5 LGPL-only corresponding source.
- [x] Independent ROM + Windows + Sources preflight: https://github.com/Blueteemo/CPyMO/actions/runs/41905633936 (PASSED; **not** a cross-platform release audit).
- [ ] macOS native ARM64 build and package from the SAME source snapshot.
  - 2026-11-04: macOS 15 job in run 41901139830 cancelled after 15m without a runner or steps.
  - 2026-11-04: macOS 26 fallback run 41903702349 also cancelled after 15m without a runner or steps.
  - GitHub Actions had service degradation and there are independent community reports of similar macOS hosted-runner acquisition failures. This is no evidence of a CPyMO build error.
  - Apple Silicon local reproducible alternative: [MACOS-ARM64-LOCAL-VALIDATION-1.2.1.md](MACOS-ARM64-LOCAL-VALIDATION-1.2.1.md).
- [ ] Cross-platform paired audit with both platforms and source ZIP, final SHA-256 ledger.
- [ ] macOS Finder/Dock actual rendering of **new** cleaned icon and single user-facing lightweight acceptance.
- [ ] Freeze one final source commit, re-audit if source changes, and publish `nds-v1.2.1` only if all release gates pass.
- [ ] Promote NDS-centered root README and update old links after release exists.
- [ ] Update existing upstream PR #60 SOURCE BRANCH and its description (both required; only after release). Preserve the upstream project's generic README when syncing the code to upstream PR.

## Restrictions

- Do not call first-pass RC artifacts a released product.
- Do not claim a green CI proves a physical Mac Finder/Dock visual test.
- Do not overwrite `nds-v1.2.0`, user game directories or save data.
- Do not ship quarantined/unlicensed FVDS source or claim FVDS playback.
- No further NDS hardware visual retest for the approved one-step rail brightness adjustment.
- Clean obsolete branches only after the Release and PR #60 are safely linked.

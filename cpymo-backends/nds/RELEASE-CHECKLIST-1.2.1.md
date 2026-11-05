# CPyMO for NDS 1.2.1 — final release gates

Status: release candidate, **not yet a public GitHub Release**.

## Approved product baseline

- [x] v1.2.1 final ROM based on test4, with Default RGB555 rail (15,26,30).
- [x] 16 console IPL colors, cached Auto palette, theme-matched readable text, 2-pixel selection rail.
- [x] Title auxiliary guide reset on in-game Restart; START launcher action; RGB555 Alpha rounding.
- [x] Conservative +1.5 dB audio gain with lookahead peak guard; no game-volume changes.
- [x] Final image design selected: NDS full square, Windows/macOS companion converter rounded, from same user-supplied illustration.
- [x] NDS 32x32/15-color banner and compiled fallback icon resources updated together, with matching source-level regression tests.
- [x] Approved round-mask desktop source integrated in d229c58; original PNG and RGBA identities remain pinned.
- [x] Desktop icon attribution and real Windows/macOS embedding audit passed at ee52c70 (run 42025940566). Repeat actual-file audit for the final Mac refinement SHA.
- [x] 1.2.1 metadata, release notes draft, and NDS documentation updated on the candidate branch.

## Final Mac refinement acceptance (2026-11-06)

- [x] Reviewed 65e12d2 fast-forwarded into the candidate workspace; frozen Mac keeps native Bundle icon, Windows route unchanged.
- [x] Approved Mac-only 80% uniform LANCZOS derivation, centered transparent padding and every stored ICNS frame audited locally.
- [x] Local research candidate: 106 complete converter tests, 12 icon tests, strict ad-hoc signature, real Finder/Dock, synthetic GUI conversion, input protection and normal restart passed.
- [x] User accepted the current visual scheme; source artwork, Windows outputs and approved NDS behavior preserved.
- [x] Historical startup delay remains undiagnosed; no speculative startup refactor or speedup claim.
- [ ] Freeze final HEAD after documentation and push normally to the candidate branch.
- [ ] Same-SHA workflow: universal ROM, Windows, macOS ARM64 and source package; native frozen tests and full regression.
- [ ] Final actual-file paired audit and SHA256SUMS, including every Mac ICNS frame and seven Windows RT_ICON payloads.
- [ ] Brief native Finder/Dock, window, quit/reopen and strict signature confirmation using the new final CI Mac package.
- [ ] Save final run/job/attempt provenance, bytes, hashes and receipt outside the source tree; stop for release review.

These unchecked items deliberately await the final run. The local research package is not a same-SHA paired CI distribution. Developer ID signing and Apple notarization remain unavailable.

## Immutable first-pass paired release candidate (historical)

- First paired CI: https://github.com/Blueteemo/CPyMO/actions/runs/41901139830
- CI snapshot: `b8b07a805edc3a6d6bf5504d6c8720e92ed547e9` (before the later documentation-only commit).
- Verified universal ROM: 935,936 bytes, SHA-256 `9046c0440109f6c196ac7af0106625bc5407fbf5128e35beab07d8cff38b8de2`.
- [x] NTR/TWL combined header, approved 02-B background, standalone ROM validation.
- [x] Earlier converter regression and prior pinned icon generation (historical; repeat after the new desktop icon integration).
- [x] Source RC ZIP, original icon and third-party attribution included.
- [x] Windows frozen build + synthetic video conversion, exact ROM pairing, FFmpeg 7.1.5 LGPL-only corresponding source.
- [x] Independent ROM + Windows + Sources preflight: https://github.com/Blueteemo/CPyMO/actions/runs/41905633936 (PASSED; **not** a cross-platform release audit).
- [ ] First-pass b8b07a8 macOS build was not completed (historical snapshot; later ee52c70 paired build passed).
  - 2026-11-04: macOS 15 job in run 41901139830 cancelled after 15m without a runner or steps.
  - 2026-11-04: macOS 26 fallback run 41903702349 also cancelled after 15m without a runner or steps.
  - GitHub Actions had service degradation and there are independent community reports of similar macOS hosted-runner acquisition failures. This is no evidence of a CPyMO build error.
  - Apple Silicon local reproducible alternative: [MACOS-ARM64-LOCAL-VALIDATION-1.2.1.md](MACOS-ARM64-LOCAL-VALIDATION-1.2.1.md).
- [x] Later ee52c70 paired audit and SHA-256 ledger passed in run 42025940566; final refinement audit remains listed above.
- [x] Later Mac native acceptance and approved refinement completed; new final CI package confirmation remains listed above.
- [ ] Formal release authorization after final candidate acceptance; no Tag/Release is created in this task.
- [ ] Promote NDS-centered root README and update old links after release exists.
- [ ] Update existing upstream PR #60 SOURCE BRANCH and its description (both required; only after release). Preserve the upstream project's generic README when syncing the code to upstream PR.

## Restrictions

- Do not call first-pass RC artifacts a released product.
- Do not claim a green CI proves a physical Mac Finder/Dock visual test.
- Do not overwrite `nds-v1.2.0`, user game directories or save data.
- Do not ship quarantined/unlicensed FVDS source or claim FVDS playback.
- No further NDS hardware visual retest for the approved one-step rail brightness adjustment.
- Clean obsolete branches only after the Release and PR #60 are safely linked.

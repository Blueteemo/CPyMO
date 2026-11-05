# CPyMO for NDS 1.3.0 release checklist

The 1.2.1 candidate was never formally released. Version 1.3.0 retains its approved behavior and artwork, and requires real new builds; old artifacts must not be renamed into this release. Publication status is recorded after validation, outside the frozen source commit.

## Fixed baseline and version sources

- [x] Approved runtime baseline 77a9af300631f2e313e95cc88bec857f2eae8e3c retained; existing main homepage integrated without changing runtime code.
- [x] NDS VERSION and release.json unified at 1.3.0. Build-generated CPYMO_NDS_VERSION and six Banner titles consume these actual sources.
- [x] Converter release_info.py, GUI footer, frozen job metadata, Mac Bundle fields and package VERSION.json consume the shared release metadata.
- [x] NDS full-square artwork uses the user-accepted warm-grey-brown frame, native 24/32/64px resources and mild small-icon detail enhancement. User reported successful visual hardware testing on 2026-11-06; candidate ROM SHA-256 d1eed5c919d01724f5c48eef4bb952f8c4fa0ab886504943cf8be0788cc15623.
- [x] Mac native Bundle route with 80% uniform transparent padding and Windows outputs unchanged.
- [x] Cache format 6 and existing supported cache/save compatibility retained; no theme/clock/audio/game-logic changes.
- [x] Current package names and workflow artifact references target 1.3.0, with formal user-facing text and no RC suffix.

## Build and acceptance gates

- [x] Freeze one final source SHA; normally push the candidate branch and build all four distributions from that SHA.
- [x] Full regression, icon checks and both native frozen converter tests pass; record actual counts and job/attempt results.
- [x] Actual ROM contains compiled runtime 1.3.0 and six 1.3.0 Banner titles; CRC, NTR/TWL header and approved resources pass.
- [x] Boot the new ROM in an emulator and capture an existing version display showing 1.3.0, or prove final ROM byte-identical to the already captured and hardware-accepted visual candidate; no new UI or extra hardware retest.
- [x] Final CI Mac package: actual Finder/Dock, 1.3.0 version, window, normal exit/reopen, arm64 and strict ad-hoc signature pass.
- [x] Actual Windows icon payloads, all Mac ICNS frames, ROM copies, source tree/submodules, FFmpeg source/configuration/licenses and new SHA256SUMS agree.
- [x] Formal file layout passes a final real-file audit without changing ROM, EXE, .app, FFmpeg or icon payloads after acceptance.

## Publication and contribution

- [x] Synchronize main without losing concurrent changes, tag the accepted source as nds-v1.3.0 and publish CPyMO for NDS 1.3.0 (not Draft/Prerelease).
- [x] Read back public Release metadata, final asset sizes/digests, tag target and download links. Preserve historical releases.
- [x] After the Release exists, update the fork homepage and actual 1.3.0 links; homepage-only commit may follow the tag.
- [x] Update existing upstream PR #60 source and description, retaining the upstream generic root README; no replacement PR, force push or self-merge.
- [x] Archive final files, audits, emulator/native version evidence and publication receipt at stable shared links.

Mac remains Apple Silicon native with an ad-hoc signature, without Developer ID signing or Apple notarization. Do not disable system security. The historical approximately 64-second first-window wait remains undiagnosed; do not claim a startup speed fix.

Historical evidence: [1.2.1 final candidate receipt](https://chatgpt.com/space/page_0870f359e4348191a58b3ae7bbf32ae5). Historical 1.2.1 documents remain unchanged; current entry is [1.3.0 release notes](RELEASE-NOTES-1.3.0.md).

## Published verification record 2026-11-06

- Formal Tag/source: `nds-v1.3.0` / `989ae58762e7b56154fbf3d024a94bd5544c27d9`.
- [Release](https://github.com/Blueteemo/CPyMO/releases/tag/nds-v1.3.0): public, not Draft/Prerelease; all five downloads checked, server sizes/digests match.
- [Final paired run](https://github.com/Blueteemo/CPyMO/actions/runs/42104899012): attempt 1, six jobs successful; 116 regression tests passed, 12 desktop-icon and 5 native-icon derivation tests included.
- Actual four-file audit equals CI audit; 981 tracked/required-submodule blobs checked; final ROM matches user hardware-accepted visual candidate byte-for-byte.
- Final Mac ARM64 package confirmed in Finder/Dock, strict ad-hoc signature, 1.3.0 footer, chooser cancel, normal quit/reopen. No Developer ID signature or notarization.
- [Existing upstream PR 60](https://github.com/Strrationalism/CPyMO/pull/60) source is `87a2a4fcc38e9083b98f1d2498e67ecfc3bcb654`; generic upstream README preserved. Upstream Nintendo DS and CI runs are action_required, awaiting maintainer approval; they have not executed and are not counted as passing. No self-merge/close.
- [Final receipt, hashes and screenshots](https://chatgpt.com/space/page_f6755bdd11848191803bba11fcb288c3). The frozen Tag and binary distributions remain unchanged; this is a documentation-only completion record.

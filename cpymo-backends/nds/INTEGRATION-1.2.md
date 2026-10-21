# CPyMO NDS / DSi · 1.2 integration checkpoint

This branch combines the independently developed **three-mode / 3DS-inspired display prototype** and **clean FVDS stream + stereo ADPCM codec components**. It is a development snapshot, **not the v1.2 release**. The previously hardware-accepted R7 branch remains unchanged.

## What is implemented

1. **Classic upper** (default): original R7 game screen above, helper panel below; touches anywhere on the auxiliary touchscreen work as A, not as upper-game coordinate projections.
2. **Classic lower**: original R7 game rendering below, helper above. Physical lower-screen touches map through `display_x/display_y/display_scale` into the real PyMO logical coordinate space, including normal choices and menus.
3. **Enhanced 3DS-like split**: scene and sprites above; dialogue, engine UI and story selection widgets below. Bottom dialogue is composited against a subdued native RGB555 background. The top scene remains visible when an engine menu is opened, and story selections are located on the physical touchscreen.
4. **Control policy**: each standalone L press cycles 0→1→2→0. SELECT alone still quick-loads; Y retains auxiliary hide/reveal, with the single-screen fallback. The selected mode is stored per-game in `save/nds-dualscreen-mode.txt` as `NDSDISPLAY2 N\n` and migrates the one-byte prototype value `1` to enhanced state 2.
5. **FVDS stream**: independent bounded C reader scans headers, length-validated video/audio payloads, exact keyframe/packet boundaries, EOF and keyframe seeks. No memory-hungry full movie buffer.
6. **FVDS ADPCM**: independent C algorithm consumes 264-byte left/right blocks, yielding 256 interleaved stereo signed PCM16 frames. Each block validates both step indexes first, has no heap allocations, and controls **no ARM7/Maxmod/GX/VRAM resources**.
7. **Audio reference CLI**: `cpymo-backends/nds/tools/fvds-to-wav.c` combines the same production C modules to decode FVDS audio into host-side PCM WAV without rewriting the source file. The NDS runtime `#movie` path **does not yet call these modules**; FVDS is still read-only preflight there.

## Actual private 159-second FVDS audio validation

- Input: user-provided `full-j1.fv` exactly 12,942,736 B, SHA-256 `8b2a560f37019b0db470d3e1077f225489d1df94b1daa9faabc3f3024a43a2e8`.
- The `fvds-to-wav` binary was built in a public GitHub Actions host workflow from the actual C sources, then run **locally against the private sample**. Neither video nor decoded audio was uploaded to GitHub.
- WAV output 30,276,652 B (44 B RIFF header + 30,276,608 B samples), 47,605 Hz, 16-bit signed stereo; 7,569,152 audio frames per channel = 158.9991 s.
- Output SHA-256 `58c1f0021715f35008a32154ed08a911c559e0e387e4ab00f633b20cf90abd81`.
- Left/right RMS 8312.85/8399.18; combined RMS 8356.13, peak 32767, 27,437 clipped samples over both channels. These independent checks exactly reproduce the handoff report's peak, RMS and clip count. This is codec correctness on a **host PC**, not proof of real DSi audio playback.
- Host build reproducibility:
  ```sh
  gcc -std=c11 -O2 -Wall -Wextra -Werror -I cpymo-backends/nds/source \
    cpymo-backends/nds/source/fvds-stream-nds.c \
    cpymo-backends/nds/source/fvds-adpcm-nds.c \
    cpymo-backends/nds/tools/fvds-to-wav.c -o fvds-to-wav
  ./fvds-to-wav input.fv output.wav
  ```
- Public synthetic-only CI checks cover: native stereo decode vectors, one combined FVDS packet+ADPCM pass, WAV structure/PCM duration, malformed audio, malformed indices, trailing data, exact indexed keyframe packet boundaries, old preference format, save/reload and C sanitizer checks.

## Explicit release blockers

- **FVDS actual video decode is NOT implemented**: j1 video requires DCT/motion compensation reconstruction. The independent file reader and ADPCM codec cannot display video.
- **FVDS runtime sound output NOT integrated**: Maxmod/Calico ARM7, 47,605 Hz stereo handling, simultaneous BGM suspension/resume, IPC/ring buffers and latency need safe shared ownership.
- **Video graphics lease NOT integrated**: CPyMO owns 2D bitmap VRAM A/C; reference FastVideoDS uses GX/3D, DMA, timers and often ARM7 FAT/audio. No external, unlicensed player code has been incorporated. FastVideoDSPlayer issue #15 is still an unresolved license request.
- **New 3-mode UI NOT hardware-accepted**: user accepted the earlier two-screen foundation and NVL1 B-skip on DSi TWL, but intentionally postponed the newer three-mode build. The improved lower-screen choices, persisted mode and touch are CI-validated only.
- **Normal NTR 4 MiB/R4 hardware is NOT accepted** for enhanced three-screen and FVDS playback. Hardware storage paths remain directional: DSpico-launched TWL can access `sd:`; console SD-launched executable could not reverse-access the DSpico TF volume during the user's tests.
- **New integrated .nds has NOT been hardware-tested**. Build CI must be green from the exact combined commit; transient devkitPro package database 403 errors can fail build before compilation and should not be confused with code defects.

No v1.2 tag, release, game asset format change, user save migration beyond the separate per-game mode preference, or public copyrighted media is authorized or claimed here. Next: original FVDS video frame decoder contract and a safe playable pipeline, with controlled resource restoration and focused DS/DSi physical acceptance.

## NDS ROM banner and quieter development CI

The 3-line `.nds` cartridge Banner metadata is generated by `ndstool -b` inside `build.py` (NOT by the launcher’s "About" UI). The third line in 1.2 test sources is now porting attribution **千岚之夏**, replacing the generic `PYMO / CPyMO`. The first two lines remain the program name and actual release metadata version (currently `1.1.0` until a formal 1.2 version bump); original upstream contributors are still credited inside the program and license notices.

Important: devkitPro/ndstool's 32x32 BMP banner writer puts input bytes directly into UCS-2 cells, so passing UTF-8 Chinese text into `-b` alone is wrong. New `rom_banner.py` patches only the 6 language-title slots in the completed version-1 banner as UTF-16LE, recomputes the Nintendo CRC16, checks ROM bounds/incoming CRC, re-reads all titles, and leaves the icon, binary and game assets untouched. `test_nds_rom_banner.py` uses a synthetic ROM for corruption, CRC, untouched sections and idempotence checks. This code is not yet confirmed by physical AKMenu display; Chinese glyph availability on alternate boot menus is also a later acceptance gate.

To address multiple GitHub Actions failure notification emails, experiment branch `push` builds are **opt-in**:
- `Nintendo DS` runs automatically on relevant pull requests and only on a 1.2 integration push changing `.github/ci-requests/nds.txt`; it retains `workflow_dispatch`.
- `FVDS Host Audio Tools` runs on an integration push changing `.github/ci-requests/fvds.txt`, or `workflow_dispatch`.
- Older synthetic/dual-card fixture workflows retain only explicit `workflow_dispatch` triggers in the integration branch.
- The main upstream `.github/workflows/ci.yml` continues to run for appropriate PRs; no repository-wide Actions were disabled. This reduces repetitive external devkitPro package mirror 403 notifications without hiding failures at actual release validation checkpoints.


## 2026-10-22 — bounded per-frame stage and deterministic LCD reset

- Added `fvds-stage-nds.[ch]`: allocation-free per-frame boundary between the production FVDS stream reader, the independently developed ADPCM decoder, and future video/audio consumers. The caller supplies short-lived callbacks; each stage reports FRAME / EOF / SKIPPED / ERROR separately. Compressed video is delivered in bounded 512-byte chunks, and 264-byte stereo ADPCM pairs are decoded to 256-frame interleaved PCM blocks; no unbounded movie packet is allocated.
- Callback contract includes 64-bit rational frame PTS and cooperative polling for B skip / lid or other host input. On early skip or decoder error the packet is finished to a bounded seek point for safe caller teardown. This stage is **not a video image decoder** and is **not yet connected to `#movie` or the ARM7 mixer**.
- Added `test_fvds_stage.py`: executes the actual production C implementation with synthetic generated packets; asserts callback ordering, 12 fps PTS, all three video frames, 47 ADPCM pairs, early skip after a completed first frame, malformed audio handling, and failed video consumer cleanup. **One intentionally requested milestone** ran the FVDS host workflow and passed: [run 39675049433](https://github.com/Blueteemo/CPyMO/actions/runs/39675049433). There was no automatic full NDS SDK build for these ordinary commits.
- Fixed an additional game-to-game display hazard: when returning to the launcher, the LCD map now explicitly restores physical main-top/aux-bottom and clears residual per-game enhanced/hidden states; starting any game first resets to known main-top before applying a saved lower-game preference. A relative `lcdSwap()` alone could otherwise accumulate state across game sessions. **This latest LCD restoration patch is pending NDS/DSi CI and real-device acceptance**, which remain deferred to the next concentrated milestone.

Keep stable R7 and version number 1.1.0 unchanged. User-approved banner third line remains `千岚之夏` as the NDS port credit, pending AKMenu-Next hardware acceptance. The next major implementation remains **video frame decode + graphics resource lease + ARM7 stereo audio consumer**, then end-to-end gameplay regression across all three modes; the present stage does not justify release tagging.


## 2026-10-22 — I/P picture prefix and rational stereo prebuffer milestone

- Independently verified the compressed picture-prefix bit: the first 16-bit word stored little-endian is consumed MSB-first; indexed I frames use a six-bit quantizer, and the private j1 fixture's first I frame gives q=30. Eight indexed I pictures and 1,900 predicted P pictures were independently counted in the 1,908-frame private movie. The rest of the picture block VLC, DCT/IDCT, reference motion and RGB555 compositing is **still not decoded**.
- Added `fvds-picture-prefix-nds.h` and verification inside the bounded frame stage: aborts an indexed keyframe that starts with a P picture before forwarding compressed chunks. Host regression covers normal predicted non-key frames, deliberately malformed keyframes and the real q30 header interpretation.
- Added `fvds-pcm-queue-nds.[ch]` plus `fvds-av-clock-nds.h`. This allocation-free, caller-owned, single-thread 16-bit stereo ring and rational timing contract has no Maxmod, ARM7, DMA, IPC or VRAM coupling; it only models the future output boundary.
- Full 159s user-provided FVDS scheduling analysis: 961 packets carry 15 ADPCM stereo pairs, 947 carry 16 pairs. At 47,605 Hz, 12 fps, 7,569,152 actual PCM frames vs 7,569,195 nominal target (short by 43 frames, ~0.9ms at EOF). A naïve immediate decode/drain scheduler introduces 255 silent underflow samples. With two packets prefetched ahead of the movie clock, an 8,192-frame (32KiB) ring peaks at 7,934 frames and avoids intermediate underflow entirely; only 43 missing samples at natural EOF require padding. **This is a host model, not measured ARM7 timing or proof that the hardware can decode quickly enough**.
- `test_fvds_pcm_queue.py` drives the same production C stream, stage, PCM decoder, queue and clock through the three-frame synthetic fixture, including a genuine two-packet preroll, wraparound, overflow refusal, silence on underrun and timing checks. The final deliberately triggered FVDS host workflow is **green**: [run 39686825379](https://github.com/Blueteemo/CPyMO/actions/runs/39686825379).
- Avoid extra notification noise: full NDS/DSi SDK CI has **not** been triggered for these source changes. Only the explicit host milestone was checked. End-to-end video/image decoding, ARM7 audio output, normal DS 4MiB resource management, B skip and all three LCD layouts still require hardware acceptance before 1.2.

Stable 1.1.0 and R7 remain unchanged; the 1.2 banner continues to show the port credit `千岚之夏` and the real, unbumped version number.


## 2026-10-22 — original CPyMO 3DS visual layout and test-session economy

Latest 1.2 enhanced display work is now based on an explicit comparison against the original upstream 3DS `main.c`, `cpymo_backend_image.c` and `cpymo_backend_text.c`. The visual focus is game-authored art and correct category partition, not decorative recreation of a 3DS system shell.

- Changed the lower-dialogue safe-area from an approximate 20%-height offset to a physical **14-pixel top anchor**, computed from actual game resolution, message-box height, speaker-font height, namebox vertical origin and letterboxing. Oversized authored artwork retains its original position to avoid further cropping.
- Normal enhanced gameplay keeps **original game-owned namebox, textbox and text**, plus a subtle dark gradient and a nonintrusive localized "A / Touch" hint **only when dialogue is active**. This has no effect in either classic mode.
- UI now splits the *same drawing categories as upstream 3DS*: `ui_bg` appears on top and `ui_element`/`ui_element_bg` below. To avoid blank upper LCDs on menus that omit a custom UI background, the top scene is also drawn beneath its UI backdrop. NDS intentionally routes story choices below (unlike upstream 3DS) for usable physical touch selection.
- Added host regression `test_nds_dual_layout.py` for several original game resolutions, nonzero name offset, letterboxed content, absent namebox and oversize art. Passed in [NDS UI Host Regression 39691998874](https://github.com/Blueteemo/CPyMO/actions/runs/39691998874), together with saved display preference and ROM Banner CRC tests.
- A milestone full SDK run [39691721143](https://github.com/Blueteemo/CPyMO/actions/runs/39691721143) was requested **once**: ARM9/ARM7 build blocked at devkitPro package database HTTP 403 before compilation. Its independent converter run initially exposed an old **test-only bug** that treated the banner CRC as an immutable byte, which has been corrected; the targeted Banner regression then passed in SDK-free UI CI. This is **not** evidence of an ARM9 compilation error, nor proof that the latest display visuals work on hardware. Avoid repeating failing SDK runs solely to produce redundant email.
- Hardware plan: aim for ~2–3 **concentrated** real-device sessions: (1) enhanced 3DS-like visual and three-layout touch test, (2) real FVDS decoded audio/video playback and B skip when integrated, (3) optional final release candidate across TWL/NTR; deviations or performance defects may require extras. The two major feature lines remain gated separately.

No DSi visual acceptance is claimed yet. Do not publish 1.2 until the actual hardware UI, real FVDS video decode and ARM7 audio path are verified. Stable R7 and current version metadata stay unchanged.


## 2026-10-22 — exact-source enhanced dual-LCD capture milestone

The user rejected fabricated high-resolution AI art as unusable for judging CPyMO 1.2 UI. The development branch now has a **fully opt-in, actual framebuffer export** rather than a fictional design mockup:

- `visual-capture-nds.h` is included in the actual `cpymo-backends/nds/source/main.c` dual-LCD draw path. Only if `<game>/save/nds-visual-capture.flag` exists at game start, the first real enhanced dialogue and first real enhanced menu each save **two exact 256×192 RGB555 framebuffer images** (top then bottom). Without the flag: no file writes, no second frame allocation and no test-mode drawing.
- The 16-byte `NDSPIX1` header contains real width/height, two-screen count and dialogue/menu kind. Each capture is approximately 192 KiB and is intentionally omitted from performance/audio timing tests.
- `tools/nds-capture-to-png.py` converts this captured RGB555 data into two native-resolution RGB PNG files **without dependencies, art generation, upscaling or simulated visuals**.
- Regression tests `test_nds_visual_capture.py` and `test_nds_visual_capture_c.py` validate both the pure-Python converter and **actual production C frame writer**, including opt-in behavior, buffer dimensions, RGB555 channels, raw header, two-screen split, per-mode one-shot guards, PNG signature/chunk CRCs and malformed files. Synthetic test pixels are used **only for automated assertions** and must not be presented as screenshots of the game.
- The dedicated SDK-independent [NDS UI Host Regression run 39694100531](https://github.com/Blueteemo/CPyMO/actions/runs/39694100531) passed, without devkitPro package downloads or a full NDS build.
- The most recent .nds for this exact capture instrumentation remains **unbuilt/unverified** because full NDS CI previously hit devkitPro package-index HTTP 403; no running emulator is available in the current execution environment. An authentic image preview can be claimed only after the opt-in pixels have been recorded by the actual DSi or working emulator in the user's next already-planned concentrated UI test, **not** from synthetic fixtures.

No update to 1.1.0 / stable R7; no new user test requested now. Continue FVDS video frame decoder work separately.

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

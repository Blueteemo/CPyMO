# CPyMO for NDS / DSi 1.2 — experimental video milestone

**Status: research only.** Base: hardware-accepted R7 branch `codex/nds-dsi-asset-cache-r7`, commit `98ac08432cbff35708570fe1f311c37010bf4992`. Existing `VERSION` and `release.json` still identify 1.1.0; no new tag, Release, or version bump has been authorized. R7 can be evaluated for a 1.1.1 maintenance release separately, but 1.2 development must not change its verified fallback.

## Grounded video results (private 2026-10-21 user handoff)

- 159 s, 256×192, 12 fps, 1908-frame opening. Original MP4: 3,169,526 bytes.
- Previous NVL1: **30,861,377 bytes**; FVDS sequential j1: **12,942,736 bytes** (58.06% reduction vs NVL1).
- j1 has exactly **92 bytes header/index + 7632 bytes frame headers + 5,129,324 bytes video payload + 7,805,688 bytes audio**. Video section = 5,136,956 bytes (78.46% smaller than previous NVL1 video section).
- Sequential j1 *encoder reconstruction*, not playback, was 36.70 dB full-screen PSNR; mid-screen crop approx. 34.36 dB; text readability and independent decoder quality remain to be tested. Do not extend j1 results to j8.
- Modified official v0.0.1 standalone diagnostic player and explicit Slot-1 ownership adjustment reached frame 1907 with reported lateCount=0 in an NTR/4 MiB melonDS 1.1 experiment. **Neither the unmodified official ROM, real R4, DSi nor CPyMO engine integrated playback passed this test.** Its hardcoded IRQ trampoline used DLDI reserved space and must never be reused/distributed.
- Current FVDS audio is 47,605 Hz stereo 132-byte ADPCM blocks, not CPyMO's 22,050 Hz mono PCM/NAR format. Do not change it by altering the container header.

**Source/provenance:** the user's private `CPyMO-NDS-DSi-视频研究交接包-20261021.zip` and its independently reviewed black-box logs, not files shipped in this repository. No reference FastVideoDS player/encoder source, copyrighted game video, or modified diagnostic ROM is committed. FastVideoDS player/encoder root LICENSE was not evident and the license-request issue remains open; verify permission before any public reuse.

## Verified integration boundaries in R7

- **ARM7:** `cpymo-backends/nds/arm7/main.c` owns Calico PMIC/RTC/touch/sound and Maxmod. Standalone FVDS also expects its own ARM7 FAT/ADPCM/FIFO scheduling: **do not transplant its second main loop**.
- **Storage:** R7 `fatInitDefault()` selects current volume and discovers games on `fat:` and `sd:` in genuine TWL mode. Open video under the current game's absolute path; do not hand ownership of Slot-1 away from CPyMO.
- **Audio:** `source/audio-nds.c` streams via Maxmod with sleep/resume management. Its existing NVL1/NVA1 movie path is 22050 Hz mono; FVDS stereo audio requires a compatible, separately scoped solution.
- **Graphics:** R7 has RGB555 framebuffer, auxiliary LCD, swap, DMA and VBlank lifecycle. FVDS reference uses 3D/GX/VRAM A/B/C/display capture; video transitions must acquire and restore these resources.
- **Interpreter:** R7 `movie-lz4-r7.h` currently reads `.nvl` and safely skips missing/unsupported video. A FVDS experiment must keep all existing `#movie` semantics and fallback intact, including end-of-file return to script.

## Planned engineering gates (not claims of completion)

1. **Host validation, started:** `cpymo-tool/nds-converter/fvds_inspect.py` is a new, read-only Python standard-library parser written without reference source code. It checks the exact FVDS header, every packet boundary, ADPCM byte accounting, keyframe offsets and trailing bytes. Synthetic tests live in `tests/test_fvds_inspect.py`. It does not decode video or audio.
2. **Isolated playback probe:** examine R7's ARM7/FIFO, timer, DMA, VRAM/3D and memory conflicts, then build a separately gated short-clip probe. Do not claim a working CPyMO player merely because a file can be inspected. Obtain reuse permission or develop an independent codec adapter before publishing actual decode code.
3. **Safe `#movie` lifecycle:** bounded allocation, pause/suspend current audio, take screen resources, render/synchronize, and use one cleanup path on natural EOF, B skip, missing/bad file, I/O/heap failure and lid close/open. Restore audio, display, timing and script progress every time.
4. **Distinct hardware gates:** NTR 4 MiB DS/R4 and TWL 16 MiB DSi/DSpico, with both `fat:` and `sd:` media where supported. Verify 1 s and 3 s clips first; only then test the full private j1 clip. Measure loaded/presented frames, lateness, audio drift/underruns, RAM peaks, I/O stalls, EOF, return-to-script and save/load regression.
5. **Opt-in converter:** until hardware and licensing pass, default remains 'video omitted'; no sudden change to the cache formats 1–6 or existing game saves. Optimize the now-major FVDS audio byte share only after audio/video integration and real-device profiling.

**Release gate:** a genuine 1.2 release must pass normal DS and DSi regression, EOF/skip/exception/sleep restoration, format verification, licensing and user approval. No automatic push to the official release branch.

## Run host checks

```sh
python cpymo-tool/nds-converter/fvds_inspect.py /local/path/full-j1.fv --sha256
python -m unittest discover -s cpymo-tool/nds-converter/tests -p 'test_fvds_inspect.py' -v
```

Expected private j1 input SHA-256: `8b2a560f37019b0db470d3e1077f225489d1df94b1daa9faabc3f3024a43a2e8`. The sample is **not** in the GitHub repository.

The suggested hand-drawn 32×32 icon is a separate art track, and the accepted R7 list/UI appearance remains untouched by this milestone.

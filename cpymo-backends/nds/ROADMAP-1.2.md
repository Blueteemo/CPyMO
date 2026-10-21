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

## Completed 1.2 preflight milestone (2026-10-21)

- `convert.py --video-fvds` is **opt-in**: copy validated, already-encoded source `video/*.fv` or `movie/*.fv` files into the ordinary format-6 sharded `nds-cache/video/XX/HASH.fv` tree, using CPyMO's existing case-insensitive FNV resource keys. Original scripts and saves are unchanged; traditional conversion **still omits video**.
- The host parser `fvds_inspect.py` validates framing, keyframe ordering, byte bounds, duration and the correct 132-byte-per-channel ADPCM block accounting. Malformed clips are rejected before publication into the finished output directory. The existing converter and new host tests pass together (**13 tests**, CI run `39534060252`).
- The independently written `source/fvds-preflight-nds.h` uses **existing `#movie`**, enabled by **`<game>/save/nds-fvds-probe.flag`**. Note that R7's `savefile()` always looks in `save/`, not the game root. It opens the selected `fat:/` or `sd:/` game cache directly and scans packet offsets/keyframe boundaries with **no frame/audio decoding**, no large image buffers, no ARM7/FIFO/Maxmod takeover and no VRAM change. A missing flag or missing FVDS retains R7's original NVL1/NVA1 path.
- Physical-test reports are individually written to `<game>/save/nds-fvds-probe-<movie>.txt` (e.g. CHECK1/CHECK3), with sanitized names, NTR/TWL mode, frame progress, data byte counts and scan time. Code is bounded to 18,000 frames, at most 512 keyframes and a 192 MiB file. Any read failure records a non-zero result code and returns control to CPyMO, just as missing videos currently safely skip.
- `make_fvds_probe_game.py` and the dedicated `FVDS Preflight Game` GitHub Actions workflow generate and convert a **synthetic** two-movie short PyMO fixture. This is an **I/O regression test**, not a codec/video playback demonstration. CI run `39534618287` passed after correcting flag placement into the converted game's save directory.
- Build CI run `39534328653` passed the original DS build, DSi experimental build and converter tests for the runtime probe. A separate private **user-facing** test package substitutes the user's provided 1-second (66,332-byte SHA-256 `27dee5ee7869497c4136d1f6335747ffbfd601cec1f5377840a0bded9cc9bf51`) and 3-second (176,252-byte SHA-256 `f55277114e167833b26656df3195207feb76d8f56351280ee34c96932f44d61f`) FVDS samples without publishing media to GitHub. Its cache hashes, sample SHA-256, manifest and ZIP CRC were checked.
- **Still unverified:** actual NDS/DSi hardware probe execution, the two `save/` reports and safe return to script. **Not implemented:** FVDS bitstream decode, onscreen video playback, stereo ADPCM playback, synchronization or ARM7/VRAM acquisition/restoration. No 1.2 release claim.

## Planned engineering gates (not claims of completion)

1. **Host validation, started:** `cpymo-tool/nds-converter/fvds_inspect.py` is a new, read-only Python standard-library parser written without reference source code. It checks the exact FVDS header, every packet boundary, ADPCM byte accounting, keyframe offsets and trailing bytes. Synthetic tests live in `tests/test_fvds_inspect.py`. It does not decode video or audio.
2. **Isolated playback probe:** examine R7's ARM7/FIFO, timer, DMA, VRAM/3D and memory conflicts, then build a separately gated short-clip probe. Do not claim a working CPyMO player merely because a file can be inspected. Obtain reuse permission or develop an independent codec adapter before publishing actual decode code.
3. **Safe `#movie` lifecycle:** bounded allocation, pause/suspend current audio, take screen resources, render/synchronize, and use one cleanup path on natural EOF, B skip, missing/bad file, I/O/heap failure and lid close/open. Restore audio, display, timing and script progress every time.
4. **Distinct hardware gates:** NTR 4 MiB DS/R4 and TWL 16 MiB DSi/DSpico, with both `fat:` and `sd:` media where supported. Verify 1 s and 3 s clips first; only then test the full private j1 clip. Measure loaded/presented frames, lateness, audio drift/underruns, RAM peaks, I/O stalls, EOF, return-to-script and save/load regression.
5. **Opt-in converter:** until hardware and licensing pass, default remains 'video omitted'; no sudden change to the cache formats 1–6 or existing game saves. Optimize the now-major FVDS audio byte share only after audio/video integration and real-device profiling.

**Release gate:** a genuine 1.2 release must pass normal DS and DSi regression, EOF/skip/exception/sleep restoration, format verification, licensing and user approval. No automatic push to the official release branch.

## Consolidated hardware acceptance package (2026-10-21)

- **User request:** do more investigation up-front so several related video tests can run in **one** card-copy / ROM launch session. The private downloadable package is delivered to the user separately and does not belong in this public source repository.
- **Core:** The existing `#movie` wrapper now writes opt-in `save/nds-nvl-probe-<NAME>.txt` logs when `save/nds-nvl-probe.flag` exists. Each log records NTR/TWL, EOF/SKIPPED/MISSING/ERROR, decoded and presented frames, elapsed wall time, measured late count and movie audio samples. The original NVL1/NVA1 behavior and automatic cleanup code are otherwise unchanged.
- **FVDS:** 1s/3s real user-provided FVDS containers exercise the current per-packet *read-only* on-device preflight and its distinct result logs. **No FVDS pixel decoder/audio output is implemented at this point.** Missing FVDS still falls back to NVL1 probing/playback when present.
- **Original generated playback:** `tests/make_nvl_playback_clips.py` creates a 1s dynamic NVL1 RGB555/LZ4/PCM video and a 3s dynamic NVA1 RGB555/LZ4/NAR2 video with audible synthetic tones, sharing original CPyMO NDS codec logic. The public fixture CI places them into NDS cache and validates manifest hashes.
- **Private long clip:** 20s (240 frames) is **prefix-trimmed** from the user's supplied 159s NVL1 file without recoding. The original packets have been LZ4-decoded and all first 240 reconstructed-frame Adler32 checks pass. The private clip itself is not uploaded to GitHub. `nvl_trim.py` and synthetic host tests make this process reproducible without shipping the copyrighted source.
- **One game, six checkpoints:** FVDS CHECK1 12-packet pass, FVDS CHECK3 36-packet pass, NVL1S 12-frame playback/EOF, NVA3S 36-frame compressed-audio playback/EOF, NVL20S 240-frame playback/B skip, and NOFILE missing-video safe return. The game plays a looping original BGM for checking sound restoration. Six log filenames can be collected offline.
- **Hardware test statuses:** Build and converter CI = passed on the latest relevant code; **all six real NDSi device experiences pending.** NTR 4 MiB movie playback and original DS/R4 storage still need separate tests. The user-accepted R7 gameplay performance and UI remain the rollback baseline.
- **No public external codec integration:** actual FVDS frame playback requires licensing/independent decoding, ARM7 streaming/IPC and VRAM capture conflict resolution; results of NVL1/NVA1 must not be cited as FVDS decoding evidence.

## Seven-checkpoint batch and true FVDS I/O (research build)

- Corrected an important validation weakness: the first FVDS preflight scanned packet headers and **seeked over** the actual audio/video bytes. The new explicitly enabled probe allocates a temporary 4 KiB buffer, reads *every* payload byte sequentially with libfat and continues pumping CPyMO audio. It computes FNV1a32 and logs `payload_read_bytes`, `payload_fnv1a32` and I/O microseconds without decoding FVDS video. The ordinary R7 audio/video paths remain unchanged.
- The host `fvds_inspect.py --payload-hash` computes the exact same checksum, and its synthetic unit test now checks full payload accounting.
- The one-session private DSi hardware package includes FVDS CHECK1 (1s), CHECK3 (3s), FULLJ1 (159s, **not played**, entirely read); NVL1S (1s RGB555/PCM animated clip), NVA3S (3s RGB555/NAR2), NVL20S (20s original 159s NVL prefix, B skip or EOF), and NOFILE missing-video return. Both clip generators and the source-independent prefix trimmer are in the public repository; private handoff media remains private.
- Private real-container transfer expectations: CHECK1 `66248 / D7819B62`; CHECK3 `176072 / 2898659E`; FULLJ1 `12935012 / 56159179` (each pair: payload bytes read / FNV1a32 hex). These are host-verified values **not yet observed on NDSi**. Full source clip totals 12,942,736 bytes with header/index and frame headers.
- Video source-only first 240 NVL1 frames have additionally been decompressed through independent host LZ4 and had all 240 reconstructed Adler32 frame hashes matched before extracting the unchanged compressed packets/audio prefix. Real device playback remains unverified until the user's test.
- The device build (NDS and DSi) and synthetic game conversion CI passed; source/private media authenticity checks, installed ROM CI provenance and ZIP CRC were verified. **No FVDS decoder** or public FastVideoDS source is being merged in this step. The seventh-checkpoint package is private and must not be attached to a public GitHub release.
- Feedback required: seven separate `save/nds-fvds-probe-*.txt` and `save/nds-nvl-probe-*.txt` reports; for dynamic clips, whether the scene visibly moves, whether audio returns, and whether a B skip and NOFILE safely return to dialogue.

## Run host checks

```sh
python cpymo-tool/nds-converter/fvds_inspect.py /local/path/full-j1.fv --sha256
python -m unittest discover -s cpymo-tool/nds-converter/tests -p 'test_fvds_inspect.py' -v
```

Expected private j1 input SHA-256: `8b2a560f37019b0db470d3e1077f225489d1df94b1daa9faabc3f3024a43a2e8`. The sample is **not** in the GitHub repository.

The suggested hand-drawn 32×32 icon is a separate art track, and the accepted R7 list/UI appearance remains untouched by this milestone.

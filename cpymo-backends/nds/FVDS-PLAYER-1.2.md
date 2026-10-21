# FVDS 1.2 — incremental playable pipeline (development contract)

Status: **compressed FVDS video is not decoded yet**, and no FVDS audio is currently routed to the NDS ARM7 mixer. This document marks the next hardware-integration seam, not a user-facing video player.

## Work completed

- `fvds-stream-nds.[ch]`: bounded file/header/index reader, sequential video/audio packets, seek and exact boundary validation.
- `fvds-adpcm-nds.[ch]`: independently implemented, tested fixed 264-byte left/right block -> 256 stereo frames PCM16 decoder. Original 159-second user sample was decoded on a host and matched the independent audio reference.
- `fvds-stage-nds.[ch]`: **new allocation-free one-frame producer**. A caller-owned struct provides a 512-byte compressed-video transfer buffer, 264-byte compressed audio block and 1024-byte interleaved PCM buffer. Each `nds_fvds_stage_step` invokes video-begin(PTS), sequential video chunks, video-end, and PCM callbacks, polling for B skip / lid/power between chunks. The codec output pointer is valid *only during a synchronous callback*. On skip/error the current packet seeks to its valid end boundary before returning; EOF is distinct.
- `test_fvds_stage.py`: independently compiles the production C source under GCC with sanitizer and tests 12-fps timestamps, three frames, 47 stereo pairs, 12,032 PCM frames, skip during frame 2, callback failure and malformed compressed audio.
- Existing standard-C host WAV tool remains for audio validation and cannot overwrite source inputs, including same inode via a symlink.

## What is deliberately NOT done

The new stage does **not** decode a video frame, display RGB555 pixels, play sound, modify CPyMO `#movie`, claim GX/VRAM, reconfigure Maxmod/Calico ARM7 or take ownership of FAT and DMA. It only creates a testable streaming point where dedicated consumers can later be attached. That distinction prevents a false 1.2 release claim.

## Next meaningful milestones

1. **Video:** design and implement a licensed or independently authored bitstream/IDCT/motion-compensation consumer of the stage's compressed chunks, with a bounded frame/reference allocation budget for **NTR 4 MiB** and a separately measured TWL path. Compare decoded RGB555 checksums and visual frames with the existing reference samples before touching NDS GPU resources.
2. **Audio:** define a ring-buffer consumer of the stage PCM callback. It needs 47,605 Hz stereo timing, explicit pause and restoration of background music, and compatibility with CPyMO's *existing* Maxmod/Calico service. A host success is not a real hardware acceptance.
3. **Display:** acquire/release graphics through a scoped lease. In classic-top, classic-bottom and enhanced modes, the movie must temporarily own only the documented buffers; on EOF, skip, malformed input, lid close or user exit, release everything and redraw original mode with no stale VRAM.
4. **Real-device gates:** one private DSpico/NDSi-TWL and R4/NTR pass for all three display modes, B skip, audio/BGM continuity, keyframe seek, and long-video memory/latency. UI/layout and ROM banner attribution remain subject to AKMenu-Next hardware review.

The stage's video callbacks **must** be backed by a real frame decoder before the test mode is advertised as full FVDS playback. Avoid copying FastVideoDSPlayer sources while source redistribution/license status remains unresolved.

Builds are requested explicitly via the existing `.github/ci-requests/...` mechanism; push-triggered per-commit CI is intentionally disabled.


## 2026-10-22: coded picture metadata + bounded PCM producer/consumer

To reduce the risk of mixing video decoder and ARM7/Maxmod ownership, two further self-contained units are implemented in this branch:

1. **Picture-type precondition, not a decoder** — `fvds-picture-prefix-nds.h` reads the high bit of the little-endian first video word as I/P metadata. The container stream now labels its current indexed keyframe; `fvds-stage-nds.c` rejects a P picture in an indexed I position *before delivering any compressed frame content*. These are independently derived bitstream checks, and cannot reconstruct DCT coefficients, motion vectors, or pixels. The private j1 159-second sample was inspected offline: exactly **8 indexed I pictures** at frames 0, 251, 502, 753, 1004, 1255, 1506 and 1757, and **1900 P pictures** in the other positions; none contradicted the key index. No private payload is committed.
2. **47,605-Hz stereo PCM queue, not a hardware player** — `fvds-pcm-queue-nds.[ch]` provides a caller-owned, single-thread ring with bounded occupancy, all-or-nothing 16-bit stereo writes, read wraparound, silence on underflow, overload refusal and counters. It has **no direct DMA, ARM7, Maxmod, synchronization or cache-coherence management**. A separate host test uses the *production FVDS stage* to decode 47 synthetic ADPCM pairs over three 12-fps frames and drains at exact integer-rational 47,605 Hz; a capacity of 8,192 stereo frames (32,768 bytes) absorbs frame-block scheduling variation with no underrun in that synthetic scenario. This 32-KiB budget is provisional until a real DS/DSi PCM audio sink is integrated and measured.

The important distinction remains: video data callbacks receive **compressed** chunks, not RGB555 pixels. A completed full FVDS 1.2 player still needs an independently implemented video bitstream decoder, sound output using an explicit resource owner, and verified restoration of original display/BGM on skip, EOF and failure. This new queue is not IPC-safe without an explicit synchronization layer.

All added code is original and contains no copied FastVideoDSPlayer implementation. New synthesized tests are shipped in the public GitHub repository; the user's original video samples remain private.


### Timing and first video-header findings on the private 159-second j1 sample

An independent, offline simulation based only on per-frame ADPCM block counts in the user's own FVDS sample (not uploaded to the repository) found:

- 1,908 pictures at 12 fps, with 8 indexed I pictures and 1,900 P pictures. The I-picture header begins with an MSB-first picture-type bit and six-bit quantizer. A sample first word `0x3c0f` in little-endian storage yields **quantizer = 30**, consistent with the research encoding setting.
- The 1,908 packets have 961 frames with **15** stereo ADPCM pairs and 947 with **16** pairs, for **7,569,152 stereo PCM frames** at 47,605 Hz. Exact 159-second nominal demand is **7,569,195** PCM frames: source capacity ends just **43 samples (~0.903 ms)** earlier than ideal, a normal short tail suitable for guarded EOF silence.
- **Naïve queue policy:** decode each picture's audio, then immediately drain that picture interval from an 8,192-stereo-frame ring. This causes a measured **255 silent-sample underflow** in the sample sequence (127 on frame 0, 127 on frame 1, 1 later). Therefore **simple same-picture push/drain is not acceptable** as the final live scheduler, even when video/picture decoder is fast.
- **One-picture audio lookahead:** prime from the first **two** compressed picture packets before starting presentation, then continually decode one future packet before consuming the next presentation interval (with A/V frame staging coordinated). With 8,192 stereo frames (32 KiB) of buffer, the whole 159-second schedule has **no overflow**, peak occupancy **7,934 stereo frames** (~31 KiB), and the **only underflow is the 43-sample EOF shortfall**. That must be padded with silence at natural EOF, not misdiagnosed as a stream failure. This is a deterministic **host simulation** of sample budgets, not a performance benchmark or an ARM7-tested implementation.

The live video decoder must therefore preserve at least one picture of compressed/audio lookahead without displaying a frame early, while the audio transport uses `nds_fvds_pcm_target()`/`nds_fvds_pcm_interval()` (rational sample clock) and never substitutes arbitrary silent priming for real samples. Maintaining the ring headroom also constrains decoder scheduling on NTR 4 MiB. The video image bitstream beyond picture flag + intra quantizer is still undecoded.

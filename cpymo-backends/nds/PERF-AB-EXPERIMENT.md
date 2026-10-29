# NDS 1.2.0 image I/O and cache A/B experiment (private branch only)

This is a diagnostic branch based **exactly on** the published `nds-v1.2.0` tag.
Do not merge, distribute its ROM as a stable release, or test on a real game's existing `save/`.
No changes to the shipped image formats, script semantics, audio playback or screen layout.

## What the opt-in patch does

- Normal execution remains unchanged unless `save/nds-loadbench.flag` exists.
- The original 32-load benchmark keeps cache **disabled** by default.
- Optional `save/nds-bench-keep-cache.flag` retains the **existing** DS/TWL memory budget instead. Never combine it with `nds-cache-stress.flag`, which still disables caching.
- Optional `save/nds-bench-audio.txt` selects three benchmark sound IDs, allowing the repository's generated synthetic game to provide actual audio.
- On benchmark completion (or specified setup/load errors), export the counters to `save/nds-loadbench-results.txt` **after** the timed loop. The pre-existing benchmark does not return to gameplay: exit the emulator or reset the device after exporting.

## Synthetic fixture (host)

From the checkout of this experimental branch, with Python 3.10+, Pillow and FFmpeg:

```sh
python3 -m pip install -r cpymo-tool/nds-converter/requirements.txt
python3 cpymo-tool/nds-converter/tests/make_dual_card_game.py /tmp/nds-perf-original
python3 cpymo-tool/nds-converter/convert.py convert /tmp/nds-perf-original /tmp/nds-perf-converted --cache-only --audio-lossless voice --compress-images --video-mode skip
python3 cpymo-tool/nds-converter/convert.py verify /tmp/nds-perf-converted
```

Make independent **copies** of the converted game for each A/B run and keep all diagnostic files inside each copy's `save/` directory. Never overwrite an installed game or a preexisting save.

In each test copy's `save/`:

```text
nds-loadbench.flag             # empty file; required
nds-bench-assets.txt           # exactly four lines:
  bg SCENE01
  chara PLAYER1
  bg SCENE02
  chara PLAYER2
nds-bench-audio.txt            # one line: BGM07 TONE1 SIGNAL
nds-image-buffer.txt           # single digit: 0, 1, 2 or 3
nds-bench-keep-cache.flag      # empty file ONLY in the caching-on comparison
```

File buffer controls apply only to NDS image cache opening:
0 = keep standard C library behavior; 1 = unbuffered; 2 = 8192-byte fully buffered (default); 3 = 4096-byte fully buffered.
These do **not** mean pre-reading entire images into RAM. When comparing file buffering, keep the cache marker absent in both runs.

Build this experimental source using the pinned devkitPro SDK:

```sh
git submodule update --init stb endianness.h
export DEVKITPRO=/opt/devkitpro
python3 cpymo-backends/nds/build.py
# For an explicitly separate TWL experiment, build.py --dsi-experiment
```

The actual runtime is distinguished by the exported `actual_twl` value; do not compare DS/NTR with DSi/TWL as if they were equal conditions. The two build modes must also not be mixed in an A/B pair.

## Interpreting results

Only accept a run where `load_bench[1] == 1`, `load_bench[2] == 0`, `load_bench[3] == 32`, with the same fixture, hardware/runtime mode and ROM build. Compare both final-frame hashes `load_bench[12:16]` **and all-eight-pass accumulated frame hashes** `load_bench[28:32]`; these do not replace real visual checks.

- `load_bench[4:8]`: cumulative image **load plus draw** microseconds per asset (eight repetitions each).
- `load_bench[32:36]`: each asset's first load+draw microseconds. `load_bench[36:40]`: maximum among repetitions 2–8, measuring worst warm pass (not p95).
- `load_bench[8:12]`: per-asset maximum microseconds.
- `load_state[9], [10], [11]`: image-cache hits, misses and evictions.
- `compressed_state[3], [5]`: cumulative decompression and checksum microseconds.
- `load_bench[16]`: maximum observed audio service interval in microseconds while playing, **not** an actual underrun count or proof of audible continuity.
- `engine_state[8], [9]`: lifetime heap peak and allocation failures; peak is not reset per run, so interpret conservatively.
- The other exported arrays preserve raw values for detailed inspection. Audio start errors make that run invalid as an audio comparison.

Run each condition repeatedly, alternate run order, and keep the TF/microSD card and scene input unchanged. A shorter loading time alone does not justify an optimization if it worsens service-gap peaks, allocation failures or hashes. No fixed improvement percentage is assumed.

## Validation status

Source checks were completed against the released tag, and the opt-in ROMs were successfully compiled with the pinned devkitPro SDK. A later synthetic-game CI completed successfully. There are **no measured emulator or hardware A/B performance results**, and no claim of improved speed or audio continuity. The official `nds-v1.2.0` tag and `main` remain untouched.

## Automatic comparison (host, not device)

Copy the exported `nds-loadbench-results.txt` from each *independent* trial to the host, using unique filenames:

```sh
python3 cpymo-backends/nds/tools/compare-loadbench.py --self-test
python3 cpymo-backends/nds/tools/compare-loadbench.py /path/to/cold.txt /path/to/cached.txt
```

The script rejects incomplete measurements, inconsistent NTR/TWL runtime, mismatched frame hashes and confounded buffer+cache A/B changes. It prints per-asset first-load, average and worst warm-pass timings, LRU hit/miss/eviction counts, audio update gap and memory counters. The self-test only verifies parsing and rejection rules with fabricated inputs: it is **not** a CPyMO performance measurement.

Experiment CI workflow `.github/workflows/nds-perf-ab-experiment.yml` is scoped to this research branch and performs the pinned devkitPro native build plus synthetic converter fixture checks. A passing build is **not** emulator or physical hardware validation.

## Final static review and disposition — 2026-10-29

This section closes the requested **theoretical/static review**, not the separate experimental A/B benchmark. No hardware acceptance or optimization release is required by this investigation.

| Area | Evidence in released `nds-v1.2.0` | Disposition |
| --- | --- | --- |
| Image loading | `source/main.c` `cached_image()` (approximately lines 199–245) selects preconverted NPI1/NPA4/NPS1/NPS2 rather than unconditionally decoding PNG/JPEG. `nds-image-buffer.txt` already selects the stdio buffering mode. | No duplicate predecode pipeline or default I/O policy change. |
| Sprite/background LRU | `main.c` approximately lines 147–186 uses image refcounts, a bounded 384 KiB/6-slot NTR cache, and a conditional 2 MiB/12-slot TWL cache. Background reuse requires TWL and sufficient memory. | Existing protection is conceptually sound; do not enlarge budgets without evidence. |
| Scene reuse | `main.c` `nds_engine_draw()` (approximately lines 718–745) reuses the immutable base layer only when draw commands/view match, rendering dialogue/overlays separately; optional `nds-scene-verify.flag` supports pixel comparison. `cpymo_backend_image_free()` invalidates the scene cache even when image refs remain; this is conservative but may sacrifice reuse. | No proven semantic failure. Preserve current invalidation rather than risk stale image pointers or hidden visual regressions. |
| Audio during I/O | `main.c` uncompressed NPI reads, `npl4-reader.h`, and `nps-reader.h` service audio in bounded chunks; `audio-nds.c` logs prolonged update gaps to RAM. | No identified unserviced image decoding loop deserving an unmeasured patch; gap telemetry is not audible-underrun proof. |
| Resource lookup | `cpymo/cpymo_package.c` has linear package index search, but native NDS cached images use independently hashed filesystem paths. | Do not graft a VNDS ZIP index onto different resource paths without profiling an actual package-heavy workload. |
| Script skip | `cpymo/cpymo_engine.c` executes interpreter steps under wait/skip and already-read safeguards. | VNDS QuickRead semantics cannot be transplanted by simply omitting draw calls; leave the behavior unchanged. |
| License | VNDS GPLv2-only code was considered solely as architectural inspiration; no code was copied. | Keep independent implementations. |

**What actually passed:**

- Native NTR and DSi-layout ROM builds: GitHub Actions run [40835958009](https://github.com/Blueteemo/CPyMO/actions/runs/40835958009), **native-build job success**; the *overall first workflow was red* only because its separate synthetic fixture initially lacked FFmpeg.
- Converter synthetic game generation and verification, 16 image variants / 12 audio files / 46 cache files; four independent A/B fixture directories; parser rejection-rule self-test: [40836196881](https://github.com/Blueteemo/CPyMO/actions/runs/40836196881), **synthetic-fixture job and overall workflow success**. The native-build job was intentionally skipped as unchanged on that later run.
- Source control: published `nds-v1.2.0` tag remained `0d1bc7a702e552e49735838a7a7eacae8ca40aeb`. Neither the stable tag nor `main` was modified.

**Limits:** The tests establish that the diagnostic branch compiles and the generated fixture verifies; they do not prove one file-buffering/cache setting is faster, that any performance regression is absent on hardware, or that audio is perceptually seamless. No A/B runs were executed. Under the agreed "theoretical analysis is sufficient; no device test" scope, this is a research result, not a proposed change to release defaults.

**Decision:** Close this cross-engine optimization investigation with **no production optimization patch and no v1.2.1 performance release**. Leave the isolated diagnostic branch unmerged; only revisit if a concrete reproduction, demonstrable correctness bug, or new engineering requirement appears. No Deep Research is necessary.

# CPyMO for NDS v1.2.1-test3 — unified device verification

This is a development-only validation ROM, not a public release. Based on the
verified test2 build. The original v1.2.0 release and upstream PR #60 remain
untouched.

## Changes from test2

- C-style theme layout: remove the full-width header color line; add a softer
  **two-pixel selection rail to every theme, including Default**.
- All 16 console IPL colors now derive the foreground, secondary, clock and
  accent text roles from their theme color, with legible pale foreground.
  The default v1.2.0 color values stay intact.
- Cache the firmware Auto palette and all 11 derived RGB555 roles once when
  loading or explicitly changing the theme. No repeated firmware/settings
  read during a 256x192 game-list redraw.
- Keep the test2 in-game Restart -> title guide reset.
- **Audio loudness experiment**: aim for +1.50 dB post-mix boost while there
  is headroom. The integer, 64-sample lookahead peak guard reduces its gain
  whenever mixed samples would exceed ±32760. It uses immediate attack and a
  gradual release to avoid hard digital clipping. This affects normal
  PCM/MP3 playback and NVL1 movie PCM/NAR2 playback. The original decoded
  sample integrity check is performed before the playback-only gain.
  No game volume preferences or Nintendo DS hardware volume are changed.

## Suggested device tests

1. Compare Default, Auto, Red, Pink, Green, Dark Blue and Yellow:
   header has no color line, all show the same narrow selection rail.
2. Browse several games quickly in Auto; compare response with Default.
3. Check foreground/secondary text color contrast on each screen,
   including English, Japanese, Traditional Chinese and Simplified Chinese.
4. Enter dialogue and choose in-game Restart; verify title helper reappears,
   then switches back to story helper.
5. Listen at the same **physical console volume** on the same game to BGM
   alone, BGM+SE, BGM+voice, voice peaks, and a movie with audio.
   Listen for pumping, abrupt changes, crackling or clipping. The boost is
   intentionally small. Pause/continue and lid sleep/resume also matter.

Host regressions verify numeric audio bounds and software theme behavior.
Only the device test can confirm analog speaker loudness and real-time audio
performance. The desktop converter icon is not part of this ROM.

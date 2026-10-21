# CPyMO NDS / DSi 1.2 · reversible 3DS-inspired dual-screen prototype

Base: `codex/nds-v1.2-video-foundation`; official R7 / `nds-v1.1.0` releases remain untouched. This feature belongs in 1.2 only after real NTR and TWL hardware acceptance.

## Modes

- **Classic (default):** unchanged R7 single-picture play on the main LCD; the other LCD shows the existing localized control/clock panel. Existing A, B, X, Y, START, SELECT and L behavior is retained.
- **Enhanced dual-screen (opt-in):** the physical **upper LCD** displays the scene (background, sprites and scene effects), the physical **lower LCD** displays the in-game dialogue box and text, or the core UI menu while open. The renderer partitions existing `cpymo_backend_image_draw_type` categories at the backend boundary, matching the *behavioral* principle of upstream CPyMO 3DS. Original 3DS code was not copied.
- **Switch:** while playing, press **L + SELECT** to toggle classic/enhanced without exiting the story, restarting, or rewriting script assets. The old quick-load remains on SELECT alone; L alone swaps LCDs in classic mode and does not invert the mandatory top/bottom roles in enhanced mode. Y still hides/unhides the auxiliary LCD; a hidden lower LCD temporarily uses classic dialogue rendering on the visible main LCD, so game text remains readable. Re-enable Y to restore enhanced layout.
- **Saved preference:** `<game>/save/nds-dualscreen-mode.txt` contains `1` (enhanced) or `0` (classic). Absent/invalid content means classic. This is separate from core game save slots and R7's `nds-ui.cfg`; no core or asset format changes. Failed preference write leaves gameplay functional.

## Implementation

- Main renderer keeps the existing framebuffer and VRAM bank owners. The top scene is drawn and DMA-published first. With enhanced mode enabled, the same RAM framebuffer is cleared, and only CPyMO's lower-screen dialogue/menu draw types are rendered before publishing to the pre-existing sub-screen VRAM. This deliberately avoids allocating another 256×192×RGB555 heap buffer.
- The low-level NDS image, rectangle and text drawing entrypoints filter by CPyMO draw types, keeping base-image caching and audio service logic independent. The dialogue's baseline shifts upward to bring the original name/textbox into the lower LCD's useful area. The core's game scripts are unchanged.
- Existing 3DS reference has a 400×240 upper and 320×240 lower screen; **DSi has 256×192 on both**. Therefore do not assume 3DS offsets/zoom/texture sizing will directly match; a local 20%-height dialogue target is provisional, to be tuned only after physical display results.
- Classic auxiliary help and minute-clock updates are suppressed while enhanced mode owns the lower screen. Y hide and mode changes trigger a complete redraw to avoid orphaned dialogue / old help panel pixels.
- All interactions remain in the established CPyMO main loop; no new libnds VRAM mapping, IRQ/FIFO ownership, ARM7 services, or exclusive video GPU use is introduced.

## Real-hardware acceptance — not yet run

Use a converted **existing** PyMO game, ideally the previously accepted 30 MB dual-card fixture or a known NVL1 demo. Test one play session:

1. Start with **classic** (delete `save/nds-dualscreen-mode.txt` only if an earlier prototype enabled dual; no other saves must be deleted). Confirm original scene, controls, time and launcher appearance.
2. While a normal name + dialogue box is visible, press **L+SELECT**. Upper: scene/characters without dialogue. Lower: the current name/dialogue box and text. Press A to advance and check multi-line dialogue/auto text.
3. Press **X** to open the CPyMO menu. Confirm menu content and controls remain usable on the lower LCD; press B to return. Check menus, choices, speaker names and scene transitions.
4. Press **L+SELECT** again. Confirm classic restores immediately, dialogue returns to original main LCD, lower help panel returns and SELECT standalone still quick-loads.
5. Turn enhanced on, then press **Y** to hide the auxiliary screen. The top LCD should still show the dialogue in temporary classic mode; press Y again to restore enhanced lower dialogue. Confirm ordinary **L** still swaps screens in classic and does not swap top/bottom enhanced roles.
6. Close and reopen the *same game*. Confirm the previously selected mode persisted. Restart test with both an original NTR build and DSi TWL build, and with the game stored in `sd:` or `fat:` according to the confirmed *directional* storage limitations.
7. Run `#movie` both in classic and enhanced. Check video is still visible, after EOF or B skip the game returns to the original chosen dual/classic mode with music intact. **FVDS remains I/O preflight only**, not playback.

**Do not ship 1.2 on CI success alone.** Any UI alignment, speaker box placement, menu functionality or performance regressions on NTR 4 MiB must be fixed first. The established stable R7 visual layout is the unchanged rollback.

## Deferred larger 1.2 goal: FVDS actual playback

The user's private video research established sequential j1 video compaction versus NVL1 on ordinary NTR, but only FVDS **read-only** path has been verified through CPyMO on DSi. 1.2 still needs licensed or original FVDS frame decoding (including texture/GX and RAM constraints) and 47,605 Hz stereo ADPCM audio integration while preserving the CPyMO Calico/Maxmod ARM7, libfat double-volume behavior, lid suspend/resume, B skip and script return. The FastVideoDS repository has no clear source redistribution license; do not copy its player implementation into AGPL CPyMO pending authorization.

Both major features are separately gated; neither may silently replace the classic visual-novel experience.

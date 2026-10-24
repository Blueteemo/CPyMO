# CPyMO for Nintendo DS — 1.0.0

NDS platform port: 千岚之夏. CPyMO: Seng Jik and contributors.
PyMO: chen_xin_ming and contributors. Engine/backend: AGPL-3.0-or-later;
standalone converter: MIT. See CREDITS.md, licenses/, the decoder's GPL
notices and the converter's font licenses for third-party attribution.

Targets original DS / DS Lite, 4 MiB RAM and FAT/DLDI Slot-1 flashcarts.
There is no DSi exploit dependency. Offline image, font and audio caches are
required; original scripts and finalized localization text remain unchanged.
The paired converter is in cpymo-tool/nds-converter and accepts S60v3,
S60v5 and Android PyMO game structures. The portable host app is Windows-only.

## Build

```sh
git submodule update --init stb endianness.h
export DEVKITPRO=/opt/devkitpro
python3 cpymo-backends/nds/build.py
```

PowerShell: `$env:DEVKITPRO='C:/devkitPro'`, then `python build.py` here.
Python 3.10+ is required for building, not for playing.
Validated SDK: devkitARM r68/GCC 16.1, Calico 1.2.0, libnds 2.0.2,
libfat 2.1.0, Maxmod 2.1.0, ndstool 2.3.1. Output: engine.nds.
build-records/ records compiler commands and diagnostics.
Source/SDK paths are normalized in compiler-emitted strings so relocating the
source tree does not embed private paths or change the ROM with the same SDK.
The installed libfat-nds 2.1.0-4 package provides the modern libdvm FAT
compatibility API; this port does not assume an older libfat implementation.

release.json is the shared engine/assistant release metadata. VERSION is
checked against it; the build generates the visible version header. This
does not change the upstream all-platform version. Cache protocol is independent
of product version: format1 uses NPI1 images, PCM16 audio and NF13/NPF1 fonts.
Existing converted games do not need reconversion for this UI/version update.
Existing save formats and locations are retained.

Both ARM9 and ARM7 use Calico ds9/ds7 specs. Do not replace the custom ARM7
with the SDK default: the PXI backlight worker uses the Calico SPI lock to
update only PMIC backlight bits, preserving standard input/touch, sound,
LED, power, microphone and wireless services. The Maxmod stream implementation
contains a documented sleep tick rebase.

## Installation and controls

Copy cpymo-nds.nds and converted pymo/games/<game> to the TF root. Keep the
flashcart kernel. Back up saves before a game update; never overwrite your
existing save/ with the converter's new empty directory. Multiple games appear
in the library. Keep ROM files outside game data folders. FAT/DLDI support
does not certify every flashcart/card/kernel combination.

| Input | Action |
|---|---|
| A / touch | Confirm / advance |
| X | In-game menu |
| B | Return/cancel in menus; hold to hide dialogue in game |
| Y | Toggle the auxiliary screen/backlight |
| L | Swap physical screens |
| R | Skip according to the game's skip policy |
| START / SELECT | Save / load standard slot1 |
| Close / open lid | Pause and mute / resume |

When the hidden auxiliary screen is the touch screen, the first touch wakes it
without advancing. Font size and read-text dimming are independent per game.
The selected game's upper title uses native 16px text; descriptions use native
12px. Lower list titles use the exact native 12px dialogue font, including
its complete 16-row storage grid and original advances. Each row builds its
own in-memory bitmap from its game's small font; selecting a game with a
different subset cannot remove its glyphs. Older on-card title/meta bitmaps
may contain the earlier rasterized font and do not control this list's style.
Game data need no reconversion.
The in-game settings include a global character-effects switch, enabled by
default and saved in /pymo/nds-global.cfg. Disabling it finishes character
fade/movement/clear transitions immediately across games; background effects
and image transparency remain enabled. It does not remove asset-loading time.
The embedded
pixel startup screen covers FAT/library loading without a minimum hold. Bitmap
VRAM is cleared before reveal; FAT failure shows an error.

## Validation and limits

The final pre-release engine (listfont-r3) was accepted by the user on an
ordinary R4 with Wood kernel on 2026-10-11. Version 1.0.0 promotes that tested
engine with release-version metadata updated; it has separate devkitARM and
stock melonDS 1.1 DS/4MiB checks. The exact 1.0.0-labelled binary is not claimed
to have been separately tested on hardware. These checks do not certify all
chapters, games or flashcarts.
Historical lid tests use a separately documented emulator scheduling fix;
they are not stock-melonDS sleep certification.

Video behavior is unconditional safe skip, even if experimental video caches
exist. The converter omits video assets and records omissions. Audio caches
are 22050Hz mono PCM16; install size is currently large. Original non-video
media are retained. Compression, optional video, DSi enhancements and broader
game compatibility are later work.

Missing original audio may be silent and is listed in conversion audits.
Missing glyphs may be boxed only under the explicit converter policy.
Packed/encrypted scripts need a separate importer. Conversion is not general
compatibility certification. Game data, user saves, SDK and emulator images
do not belong in this source directory or the upstream contribution.

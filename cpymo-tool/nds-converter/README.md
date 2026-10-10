# Nintendo DS game conversion assistant

NDS port: 千岚之夏. Standalone converter and GUI code: MIT. Fonts and bundled
dependencies retain their own licenses. The engine is AGPL-3.0-or-later.

The DS/DS Lite backend requires offline caches. Input can be a PyMO game folder,
ZIP or APK with UTF-8 text scripts and `gameconfig.txt`; nested ZIPs are supported.
S60v3, S60v5 and Android describe the **input game data**, not the host OS.
Packed/encrypted scripts and other engines are not supported.

The Windows portable distribution includes Python, Pillow and an audio-only
FFmpeg executable. Players do not install them separately. Select the original
game package, select a new output location, then click conversion. The output
contains `cpymo-nds.nds`, `pymo/games/<game>` and an installation note.

Original inputs and scripts are not edited. Existing saves are not imported or
overwritten. Images become DS-sized RGB555 caches and audio becomes 22050 Hz
mono PCM16. The current format favors playback speed over storage size.
Original non-video media are also retained; reducing that duplication and audio
cache size is future work. Static conversion success does not certify an entire
game, every script command or all flashcarts.

Video assets under movie/video directories or with recognized video extensions
are omitted from new output. Scripts retain their movie commands, which the
standard NDS build safely skips. `nds-cache/manifest.json` records omissions,
tool versions, hashes, missing resources and other compatibility warnings.
Missing audio is recorded and may play silently; finalized text is unchanged.

## Developer usage

Python 3.10+, `pip install -r requirements.txt`, and FFmpeg are needed when
running from source. The portable Windows app has been tested; native portable
macOS/Linux distributions are not provided. The CLI is available for those hosts.

```sh
python convert.py convert /path/to/game /path/to/new-output --ffmpeg /path/to/ffmpeg --missing-glyph-policy box
python convert.py verify /path/to/new-output
python app.py
```

The default CLI missing-glyph policy is `error`; the explicit `box` option keeps
the original script and uses a visible replacement glyph, with an audit record.
The GUI defaults to this documented box policy. Keep the supplied `fonts/` and
their licenses with the source. Source version metadata is read from
`cpymo-backends/nds/release.json`; do not copy only this directory out of the repo.

Build the NDS ROM first. On Windows, install `requirements-build.txt`, then:

```sh
python build_portable.py --ffmpeg /path/to/ffmpeg.exe --output /path/to/new-portable
```

It embeds the same ROM and release metadata as the output root and verifies
their hashes. Add player docs/licenses and complete corresponding source before
publication. FFmpeg runs as a separate process; it is not linked into the MIT
converter. A minimal FFmpeg build script is under `tools/`; its official source,
configuration, toolchain and runtime notices must accompany binary releases.

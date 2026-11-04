# CPyMO for NDS 1.2.1 — local Apple Silicon release-candidate validation

**Fallback for GitHub-hosted macOS ARM64 runner acquisition failures.**
This document prepares a repeatable native macOS 26 (or compatible Apple Silicon
macOS) build, with the exact already-verified 1.2.1 ROM and the already-reviewed
16px-cleaned desktop icon. It does **not** claim local tests have run.

## Immutable source and scope

- Build source commit: `b8b07a805edc3a6d6bf5504d6c8720e92ed547e9` (same source as first verified Windows and source RC files).
- GitHub Actions ROM artifact: run `41901139830`, `CPyMO-1.2.1-UNIVERSAL-VERIFIED-ROM-RC`.
- ROM byte count: **935,936**. SHA-256:
  `9046c0440109f6c196ac7af0106625bc5407fbf5128e35beab07d8cff38b8de2`.
- PyMO original icon source remains unchanged. Approved cleaned RGBA pixel hash:
  `3534161a2e9594802247f735c5ed70eedab0a55e40bce9fa3860a0f0d7f174f1`.
- No need to build another `.nds` or rerun DS/DSi hardware acceptance.
- **Build in a new work folder, not on an existing user game, save, or project directory.**

## 1. Prepare the isolated clone and verified ROM

On an Apple Silicon Mac, use a native ARM64 terminal with Xcode Command Line
Tools, Python 3.12, Git and GitHub CLI. GitHub CLI may ask you to sign in to
download Actions artifacts; do not put a personal access token into scripts or
send it in chat.

```bash
uname -m                              # must say arm64
git clone https://github.com/Blueteemo/CPyMO.git CPyMO-1.2.1-Mac-RC
cd CPyMO-1.2.1-Mac-RC
git checkout --detach b8b07a805edc3a6d6bf5504d6c8720e92ed547e9

gh run download 41901139830 \
  --repo Blueteemo/CPyMO \
  --name CPyMO-1.2.1-UNIVERSAL-VERIFIED-ROM-RC \
  --dir .release-rom
shasum -a 256 .release-rom/cpymo-nds.nds
```

The digest **must** equal the SHA-256 above. Stop on any mismatch. If the
GitHub artifact is not available, a separately obtained `cpymo-nds.nds` is
acceptable only after verifying its exact hash.

## 2. Build FFmpeg 7.1.5 from the corresponding LGPL-only source

```bash
python3.12 -m venv .venv
source .venv/bin/activate
python -m pip install -r cpymo-tool/nds-converter/requirements-build.txt
bash cpymo-tool/nds-converter/tools/build_ffmpeg_macos_lgpl.sh macos-ffmpeg-lgpl
lipo -archs macos-ffmpeg-lgpl/bin/ffmpeg       # must include arm64
```

The build script retains the original FFmpeg 7.1.5 source archive, configure
flags, license report, and binary SHA-256. It rejects GPL/nonfree builds. If
compilation fails, preserve the logs; **do not substitute an unknown prebuilt
FFmpeg** merely to obtain a green result.

## 3. Build the native .app and run the real frozen-program smoke test

```bash
python cpymo-backends/nds/tools/verify-release-artifact.py \
  --rom .release-rom/cpymo-nds.nds

python cpymo-tool/nds-converter/build_portable_macos.py \
  --rom .release-rom/cpymo-nds.nds \
  --ffmpeg macos-ffmpeg-lgpl/bin/ffmpeg \
  --target-arch arm64 \
  --output macos-rc \
  --require-desktop-icon

python cpymo-tool/nds-converter/tests/smoke_macos_portable.py \
  --portable macos-rc \
  --ffmpeg macos-ffmpeg-lgpl/bin/ffmpeg \
  --target-arch arm64

codesign --verify --deep --strict --verbose \
  macos-rc/CPyMO-NDS-Converter.app
lipo -archs macos-rc/CPyMO-NDS-Converter.app/Contents/MacOS/CPyMO-NDS-Converter
shasum -a 256 macos-rc/cpymo-nds.nds
```

The frozen smoke test uses a **synthetic** PyMO fixture to validate MP4→NVL1,
missing glyph fallback, exact paired ROM, source-file integrity and bundled
FFmpeg license configuration. The packager also inspects the real `.app`
bundle icon, Info.plist, architecture and bundled binaries. A successful
synthetic test does **not** replace GUI visual inspection.

## 4. One lightweight Finder / Dock inspection

Open the newly built `macos-rc/CPyMO-NDS-Converter.app` in Finder (not a
different installed icon from previous releases):

1. Use Finder icon view at normal and larger sizes on light and dark surfaces,
   paying special attention to the **top edge and left-side middle**. There
   should be no dark outer border; internal character outlines must remain.
2. Launch from Finder, confirm the **correct single Dock icon** and the
   version **1.2.1** in the app metadata. No duplicate app icon.
3. Confirm one clean window launch, a brief synthetic/owned-game ZIP or folder
   conversion, and no Tk overlap/ghosting regression. Do not use or upload
   commercial games for automated public CI.
4. Verify a sleep/wake or window reopen doesn't change the icon; screenshots
   are useful only to document the actual Finder and Dock view.

The app is ad-hoc signed for internal checking; it is **not** signed with an
Apple Developer ID or notarized. If macOS Gatekeeper blocks it, first verify
origin and SHA-256 and use only Apple's official **System Settings → Privacy &
Security → Open Anyway** flow if you trust the build. **Never disable SIP,
Gatekeeper globally, or remove all quarantine controls.**

## 5. Archive and communicate acceptance

The local package is **a candidate only**. Before any public Release, collect:

- Mac OS version/model, `uname -m`, chosen source commit, app/FFmpeg hashes,
  synthetic smoke result, and Finder/Dock PASS/FAIL with screenshots if useful.
- Final `.app` packaged with original ROM, `VERSION.json`, README,
  `ICON-ATTRIBUTION.txt`, `PYMO-ORIGINAL-MIT-LICENSE.txt`, and
  `FFmpeg-Corresponding-Source/`. Use `ditto` for preserving macOS symlinks.
- Independent **four-asset** cross-package ROM/FFmpeg/license/icon audit against
  the existing Windows and sources RC from the same source snapshot.
- A final frozen source commit and repeat asset audit if project files change
  before formal release.

Do not publish an unchecked local ZIP, share private tokens, touch existing
game saves, or mark the GitHub Release as completed before full acceptance.

# CPyMO for NDS 1.2.0 — RC provenance and public-distribution gate

This is the release-engineering record, **not** the public GitHub Release. Do not publish the current Windows/macOS RC ZIPs solely because CI passes.

## Provenance verified (2026-10-27)

- RC source commit: `9522869a` (branch `fix/nds-v1.2-pico-acceptance-20261027`). Subsequent `159ded8c` updates affected release/converter documentation only.
- Successful paired workflow: https://github.com/Blueteemo/CPyMO/actions/runs/40490746670
- Jobs: final-rom, converter-regression, windows-rc, macos-rc, and source-rc all **passed**.
- NTR / TWL: **one universal ROM** `cpymo-nds.nds`, **924672 bytes**, SHA-256:
  `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31`
- Approved RGB555 artwork embedded **exactly once**; its 98304 pixel bytes have SHA-256
  `92d91514ac12ccab6037a86a71e0c4cf1047917d15de59b91bcc8e4bb4d7d1b2`.
  The ROM has no placeholder image marker.
- The Windows frozen worker tested synthetic MP4→NVL1 with an exact paired ROM; the macOS Intel frozen worker tested default NVL1, glyph policy `box`, source immutability, and ROM pairing. See CI logs.
- The selected **second** PyMO long logo exists at `assets/branding/pymo-long-logo.webp` and is the only image planned for the new NDS README header. The approved background lives at `cpymo-backends/nds/assets/nds-dual-bg.rgb555`.
- User's DS/NTR and DSi/TWL real-machine acceptance was completed *before* packaging. Do not repeat those tests merely because docs or packaging change.

## Still needed before the **public** Release

1. **macOS user-local Finder/GUI check.** The current RC is unsigned and was only tested as a frozen worker on Intel macOS CI. Require the owner's separate DeepSeek report before including the Mac installer in the public release. Do not disable or recommend disabling Gatekeeper or SIP. Apple Silicon is not automatically certified by Intel CI.
2. **Exact FFmpeg binary provenance on each OS.** The Windows CI obtains `ffmpeg.exe` from the runner or Chocolatey; macOS uses an executable from the `imageio-ffmpeg==0.6.0` wheel. These can differ in version, options and effective LGPL/GPL status. Record `ffmpeg -version`, `ffmpeg -buildconf`, binary SHA-256, vendor/source URL and enabled dependencies for the **actual shipping executable** on each OS.
3. **Corresponding source and notices.** `imageio-ffmpeg` as a Python wrapper uses BSD terms; **the bundled FFmpeg executable has its own LGPL/GPL duties**. Do not mistake the wrapper's PyPI license for the binary's license. See https://ffmpeg.org/legal.html. For each distributed executable, provide the exact corresponding source and build configuration, including any required external component source, a discoverable source link or archive delivered appropriately alongside the binary, and license/attribution texts. Merely linking to latest FFmpeg master or only zipping the CPyMO repository is not proof of compliance. Do not promise a particular FFmpeg license until inspecting its actual build configuration.
4. **Corresponding CPyMO source.** Preserve full original and NDS MIT/AGPL licensing and any font/submodule notices; ensure source archive matches the final release commit and contains no personal/private game/media inputs.
5. **Final freeze, SHA256SUMS and release body.** When the owner-local macOS test and the notices/source gate have passed, regenerate packages *from the chosen immutable source commit*, verify all paired ROM hashes, record exact asset sizes/hash and update 1.2 vs 1.1 release notes.
6. **Post-release only:** promote the dedicated root README (with the selected second logo) to the default branch; then update upstream PR #60 body against the actual public release. Neither operation should predate the Release.

## Sources

- https://github.com/Blueteemo/CPyMO/actions/runs/40490746670
- https://ffmpeg.org/legal.html
- https://pypi.org/project/imageio-ffmpeg/0.6.0/

This is a compliance/engineering checklist, not legal advice. If accurate FFmpeg source provenance cannot be provided in time, delay the affected desktop binary release or replace that component with a compliant build; do not silently distribute it.

# CPyMO for NDS 1.2.0 — 正式发行附件与历史候选清单

## 当前正式版附件（2026-10-28 桌面界面维护）

本节为当前 GitHub Release 的**正式下载校验记录**，与下文历史 RC 不同。发行页面：[nds-v1.2.0](https://github.com/Blueteemo/CPyMO/releases/tag/nds-v1.2.0)。

| 发行附件 | 字节数 | 当前 SHA-256 |
|---|---:|---|
| `cpymo-nds.nds` | 924,672 | `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31` |
| `CPyMO-for-NDS-1.2.0-Starter-Windows.zip` | 57,836,907 | `60033ee444ea3833bd45ec4f2e3572264fdfd91bff6cee54f2f99c5e299f1a52` |
| `CPyMO-for-NDS-1.2.0-Starter-macOS-arm64.zip` | 55,282,366 | `e55cceb41467afe3732e6febac5d6b65ee7c2f3a65515a84e28c8f48567ac52d` |
| `CPyMO-for-NDS-1.2.0-Sources.zip` | 23,354,961 | `7875c82b3fe48514a309b5e32ead0405701297b3ab342c54264318ef85c7f589` |
| `CPyMO-for-NDS-1.2.0-SHA256SUMS.txt` | 394 | `299ec95148586f629ee4a80da59e073e340eac6f60791c95d5be3a7ec77eaf71` |

本次变更：转换器空闲时隐藏不确定进度条、游戏版本列表按数量调整高度并按需显示滚动条、视频选项宽度随窗口调整。版本号继续为 **1.2.0**，不改变引擎或游戏转换算法。

- **当前桌面安装包与对应源码的来源提交：** `3a78ec8f8dcc29edaac58751dbdc2bde67255f0a`。
- **完整构建／跨资产审计：** [Actions 40621518621](https://github.com/Blueteemo/CPyMO/actions/runs/40621518621)，所有构建、回归和配对审计作业成功。
- **正式附件替换及再次校验：** [Actions 40622711801](https://github.com/Blueteemo/CPyMO/actions/runs/40622711801)，对初始附件进行备份，按审核通过的审计记录替换四份附件并对照 GitHub 返回的哈希。
- **原始发行标签：** `nds-v1.2.0` 仍指向首发提交 `0d1bc7a702e552e49735838a7a7eacae8ca40aeb`；这是首发历史快照，不等同于本次桌面 GUI 的来源提交。
- **NDS ROM 原件：** `cpymo-nds.nds` 原发布附件 ID `682389749`、大小和哈希均未改变。无须重复掌机实机验收。
- 只有 **Windows、macOS ARM64、Sources.zip、SHA256SUMS.txt** 四个附件被替换。旧版 ZIP 不再作为当前正式附件下载，初发哈希保留如下。

## 首发正式版附件快照（2026-10-27，历史校验）

| 首发附件 | 首发 SHA-256 |
|---|---|
| `cpymo-nds.nds` | `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31` |
| `CPyMO-for-NDS-1.2.0-Starter-Windows.zip` | `8e85611033cf89fa12cb89c3727a397b98db083fb12cb29a5f818eef6a35d581` |
| `CPyMO-for-NDS-1.2.0-Starter-macOS-arm64.zip` | `975500238bb0050e0da92bdbc6d41f37e30098f0861bc8d9c015fde8298df7db` |
| `CPyMO-for-NDS-1.2.0-Sources.zip` | `67fb74c8a0a8b2b6b87aa60709fb65dfbdc0644f7744b39c58e28c9d5ef55760` |
| `CPyMO-for-NDS-1.2.0-SHA256SUMS.txt` | `5939ef5217bf3ea918edd9033013ccd0968aeea80cd03adaadc091a202ef5d7c` |

以上数字是首次公开发布时的历史身份记录，不是当前安装包应使用的校验值。

---

## 历史 RC 验收记录（保留原始记录）

以下所有“未发布”“等待验收”等表述均属于当时的候选阶段，不能覆盖上面的当前正式发行状态。

**Status: RC review only, not published.** Built by [GitHub Actions run 40494654625](https://github.com/Blueteemo/CPyMO/actions/runs/40494654625) from source commit `213d756a5467d77b7da3a52b142337f2946e7bec`. All five jobs succeeded. Files below were downloaded from that run, unwrapped from GitHub Actions' artifact ZIP and independently checked for archive integrity and paired ROM hashes.

| Artifact | Byte size | SHA-256 |
|---|---:|---|
| `cpymo-nds.nds` (the same ROM stored locally as `cpymo-nds_1.2.0_approved_universal_RC.nds`) | 924,672 | `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31` |
| Windows Starter RC (`CPyMO-for-NDS-1.2.0-Starter-Windows-RC.zip`) | 57,686,077 | `9859815c8fa5174e637da418236b24e84e0fe6590992439d7414056c9d59a055` |
| macOS LGPL Starter RC (`CPyMO-for-NDS-1.2.0-Starter-macOS-LGPL-RC.zip`) | 54,992,313 | `d99ca80e64ecdb098cb4e116f4eea7625109dbb9869c597e70dd6f4176168546` |
| CPyMO corresponding source RC (`CPyMO-for-NDS-1.2.0-Sources-RC.zip`) | 21,775,598 | `ee8e067863cf2007a21df97204313946fafc96fb57a8dfee47380da7cea010c3` |

Verification:
- Both Windows and macOS installer archives contain two readable NDS ROM copies, each exactly 924,672 bytes with **identical** SHA-256 matching the above universal ROM.
- The universal ROM contains one approved embedded 02-B background: RGB555 pixel SHA-256 `92d91514ac12ccab6037a86a71e0c4cf1047917d15de59b91bcc8e4bb4d7d1b2`. No placeholder art may ship.
- The Windows and macOS archives both carry **the identical official** `FFmpeg-Corresponding-Source/ffmpeg-7.1.5.tar.xz` (11,050,340 bytes; SHA-256 `de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f`), captured build configuration, license output, binary and source checksums. FFmpeg is built with `--disable-gpl --disable-nonfree`. Windows frozen conversion tested an original synthetic MP4 into NVL1; the macOS frozen worker tested default NVL1, box fallback and source immutability.
- The macOS app is **not signed with Apple Developer ID or notarized**. `macos-15-intel` CI success is *not* a Finder GUI or Apple Silicon test. A separate user-local DeepSeek test is pending.
- macOS FFmpeg may be ad-hoc codesigned by PyInstaller, so the executable in `.app/Contents/Frameworks` need not hash identically to the source-built input. Version 1.2's macOS packager now records both hashes separately for subsequent RC builds.

**Important non-final constraints:** The run predates a later packager metadata-only adjustment (`8b17e7f`). This package set therefore remains an immutable RC snapshot, not the final release. Regenerate all public files and final SHA256SUMS from one frozen commit after local Mac acceptance; do not silently rename these RC hashes to final Release assets. Keep the prior macOS prebuilt `--enable-nonfree` candidate entirely out of public distribution.

Release order: resolve owner-local Mac review; final source/license/notices audit; freeze; rebuild all, compare checksums; publish GitHub Release `nds-v1.2.0`; then promote the second PyMO logo and bespoke NDS README to repository root; lastly revise upstream PR #60 body using real Release link.

## Latest bilingual ARM64 / Windows RC cross-platform audit (2026-10-27)

All five jobs succeeded on [GitHub Actions run 40505159429](https://github.com/Blueteemo/CPyMO/actions/runs/40505159429), source commit `83c0b6e9bd9118780c962508f1db50d6e3a2d041`. This is the **paired ARM64-only macOS + bilingual converter** candidate offered for local DeepSeek Finder acceptance.

The downloaded GitHub artifact ZIPs each contain one nested distribution ZIP. Numbers below are for **nested distribution files**, not GitHub transport wrappers:

| Candidate distribution | Exact bytes | SHA-256 |
|---|---:|---|
| `CPyMO-for-NDS-1.2.0-Starter-macOS-arm64-RC.zip` | 53,173,599 | `84af5c2c78f45527c355d8febe104e9a3e101527c63773a48f27ba4f14f224c1` |
| `CPyMO-for-NDS-1.2.0-Starter-Windows-RC.zip` | 57,696,706 | `04d1676f07f61cd74c3cf5be3996d692002ae186a00b4a66b045120dd73c9881` |
| `CPyMO-for-NDS-1.2.0-Sources-RC.zip` | 21,789,169 | `c62afd9eca8e89049f7d11d927429cde15600f63c1b282b9d30172d517816d34` |
| `cpymo-nds.nds` (ROM transport) | 924,672 | `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31` |

Additional independent archive audit:
- All nested ZIPs passed integrity tests; Windows and ARM64 Mac both contain matching installed and bundled ROM **regular files** with the approved SHA-256.
- In the macOS `.app`, `Contents/Frameworks/cpymo-nds.nds` is a **26-byte symbolic link** to `../Resources/cpymo-nds.nds`, not an extra ROM. Hashing that link's literal target path falsely reports a mismatch. Likewise `Contents/Resources/ffmpeg` links to `../Frameworks/ffmpeg`.
- Both desktop RCs contain the **same original** FFmpeg 7.1.5 source archive: 11,050,340 bytes, SHA-256 `de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f`.
- macOS `VERSION.json` says `macos_architecture: arm64`, `rosetta_required: false`, and separately records FFmpeg input and post-bundle hashes. The owner-local Finder/Gatekeeper experience is *not* yet attested.
- The matching CPyMO source archive includes `ui_i18n.py`, `tests/test_ui_i18n.py`, and the proposed NDS-specific README. Intel Mac native binaries are excluded by the ARM64-only release policy.

**The RC is not a published release.** The source snapshot predates later QA/documentation-only commits; final public asset hashes require a frozen corresponding-source commit and cross-package audit. Upstream PR #60 must only be revised *after* the actual Release.

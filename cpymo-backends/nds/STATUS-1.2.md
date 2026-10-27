# CPyMO for NDS — 1.2 全局进度与发布门槛

更新时间：2026-10-23。性质：实验分支动态状态表；**不是**发行说明或完成声明。
集成源自 `codex/nds-v1.2-integration`。2026-10-27 的独立验收修复分支将开发版显示提前统一为 1.2.0；公开稳定版仍为 1.1.0，R7 保持可回退。**版本字符串更新不代表正式发布。**


## 2026-10-23 — Final DSpico (TWL) / R4 (NTR) shared-game candidate

The user approved the final *two genuine runtime modes* acceptance plan:
DSpico via DSi/TWL and R4 via DS/NTR. We built a **single 39.37 MiB original
PyMO stress game** with 12 scene panoramas, four sprites, eight ~83s BGM
tracks, voice/SE and four FFmpeg-derived NVL1 clips (2/9/3 silent/24s).
Full title selection, first-story dual-screen transition, subsequent
choices, quicksave/load and L/Y/X operation are included. Source script
is byte-identical after conversion; 0 unknown directives, 0 missing assets,
0 unconverted videos. No dummy padding or commercial test media.

Final integrated [CI #39865964204](https://github.com/Blueteemo/CPyMO/actions/runs/39865964204)
**all jobs success**: game generation/verification, separate NTR and TWL
compiled-memory-profile ROMs, combined ZIP with per-file SHA256 for 84 files.
The review ZIP contains an **additional compact CPYMO12_NVA1_CHECK game**
produced with the same public converter in `--video-mode nva1`, with an
original audiovisual 2-second movie and silent 3-second movie. The main
39.37 MiB game remains strictly NVL1. This lets the user's final DSpico
and R4 sessions also validate the independently implemented NVA1/NAR2
video path without duplicating the entire long-form dataset.
The actual combined artifact is
`CPyMO-1.2-DSpico-TWL-R4-NTR-FINAL-STRESS-REVIEW`. Original ROMs
use devkitPro Calico's DSi-compatible header unit code `2` in
**both** compile modes; this flag alone does not show TWL execution.
The real acceptance must see runtime `DSi 模式 · 16 MiB` via DSpico
and `DS 模式 · 4 MiB` via R4. The two ROM binaries differ in SHA,
with the same data copied to two cards. Detailed six-step human
checklist: [VALIDATION-1.2-DUALCARD-FINAL.md](VALIDATION-1.2-DUALCARD-FINAL.md).

**Important:** This is a test candidate, NOT physical validation or a
public release. Physical two-card, GUI acceptance, public licences/
corresponding FFmpeg source, public release/changelog approval, and
NVA1 physical acceptance through the supplied sidecar still require release decisions.
Stable 1.1 and R7 remain untouched.

## 2026-10-23 — First actual Windows video-converter EXE accepted

**Critical release blocker discovered and fixed:** original Windows Starter's
\`tools/build-ffmpeg-audio.sh\` built FFmpeg with video decoding and swscale
disabled; the new source-video encoder could not possibly work in such a
portable package. \`video_nvl.validate_ffmpeg\` now interrogates the FFmpeg
filter/decoder tables (not \`-version\`) and refuses audio-only builds before
writing a new video. \`build_portable.py\` runs the same gate **before packaging**
to prevent false "MP4 supported" products. The validator accepts differences
in FFmpeg 8 filter flag table widths / stderr output and has independent
mocked tests. Original 1.1 packages and games are not changed.

**Actual Windows executable:** a new opt-in GitHub Actions Windows
PyInstaller job packages the same-run NTR ROM and a video-capable FFmpeg,
then invokes the *frozen* \`Converter.exe --job\` on an original synthetic
PyMO game including an audiovisual MP4. Real result on Windows:
\`frozen_worker=PASS\`, \`ffmpeg_video=PASS\`,
\`format=NVL1\`, \`frames=14\`,
\`rom_pair=MATCH\`, \`source_preservation=PASS\`.
Cache manifest, scripts, NDS ROM hash and source preservation are checked
by the smoke script. All three jobs **success**:
[CI #39860993871](https://github.com/Blueteemo/CPyMO/actions/runs/39860993871),
with a 14-day development artifact
\`CPyMO-1.2-Windows-Portable-VIDEO-REVIEW-NOT-RELEASE\`.
NTR/TWL ROMs and full Linux converter suite passed in the same run.

The temporary CI artifact is **not the public 1.2 release**. It has only a
headless frozen worker smoke test, not a human GUI acceptance or independent
R4/NTR video performance test. Before public binary redistribution attach
all appropriate FFmpeg third-party licensing/notice and corresponding-source
materials, audit the distribution layout/launcher, finish NTR/DSi game/save
regression and consolidated versioning. Keep the public stable release at 1.1.0 until
release gates close. Do not require the owner to keep retesting every
small UI-only patch.

## 2026-10-23 — Unified opening title + compact save/load help

Owner requested two additional UI decisions after the successful DSi video
exercise, without another immediate device test:

- Classic auxiliary LCD shows **START：快速保存  SELECT：快速读取**
  together on line y=158, with y=176 left empty unless an action/status
  toast arrives. Translated English/Japanese strings deliberately use shorter
  labels to fit 256×192. Original clock (x196,y96), title/icon (x196,y40),
  game mode labels, and save-slot indication in *action feedback* retained.
- **Enhanced dual-screen does not split the game's opening/title menu.**
  The startup authored image/selection remains entirely on upper LCD with
  lower auxiliary help until first real story dialogue (#say, without
  overlaid core UI or choice). Then existing top-scene/bottom-dialogue
  dual-screen routing takes over. This is deliberately session-based rather
  than hardcoding any PyMO script filename. It does not edit original scripts,
  input mappings, mode persistence, actual story choices, or saves.
  A game with unusual title scripts that display narrative-style #say
  before the title may need separate follow-up evidence, rather than a
  claim of universal menu detection.

**Evidence:** standalone production-C title gate, host state-machine test
and updated full production display-mode/ghost-click regression.
[UI host #39857531941](https://github.com/Blueteemo/CPyMO/actions/runs/39857531941)
**passed**. [NTR/TWL SDK plus complete converter
#39857605866](https://github.com/Blueteemo/CPyMO/actions/runs/39857605866)
**both jobs passed**. These exact small UI changes have not undergone a
fresh DSi visual acceptance; bundle into later meaningful 1.2 hardware
candidate instead of sending successive minor ROMs.

## 2026-10-23 — DSi video demo hardware feedback and multilingual font correction

User reports the independent 1.2 DSi video conversion demo **runs without observed gameplay issues**. This is first physical confirmation of the actual source-MP4 → NVL1/NVA1 converter-backed #movie demo (including its user-facing playback flow), not a broad compatibility or NTR performance certificate.

User then identified a locale-dependent auxiliary help bug: simplified "增强双屏" printed as "？？？屏" in demo and Japanese "強化2画面" as "？化2画面" in other games. Root cause confirmed by comparing ROM `ui_fallback_codes` and the converter's game-specific pixel-font subsets: codepoints 增/强/双 and 強 (plus five other published UI literal characters) had never been inserted into the built-in fallback ROM table; behavior depended on incidental glyphs in each game's own cache.

Fixed both layers without touching video, scripts, or UI geometry: ROM's OFL Fusion/Unifont glyph arrays include all nine missing official UI glyphs (for existing games without reconversion), converter's mandatory `UI_CODEPOINTS` includes them for future conversions. New `test_nds_ui_font_coverage.py` enforces all published four-language UI string coverage even with an empty game cache. [Host regression #39855409830](https://github.com/Blueteemo/CPyMO/actions/runs/39855409830) **success**; [NTR/TWL SDK + full converter #39855507624](https://github.com/Blueteemo/CPyMO/actions/runs/39855507624) **both jobs success**. Fresh DSi/TWL font-fix ROM built and packaged for optional device confirmation; that exact patched ROM has **not yet** been independently hardware inspected. Legacy converted source/saves need no replacement.

## 2026-10-23 — First real ordinary-media NVL1/NVA1 converter milestone

**Public, independently implemented video encoder integrated on this 1.2
branch** (not the private FVDS decoder): \`video_nvl.py\` invokes FFmpeg
to decode ordinary input MP4/AVI/MKV/MOV/etc to stream-bounded 256×192
RGB555, normalized 12 FPS by default, with native NVL1-compatible packet
framing, four-second reset/key pictures, optional LZ4 XOR delta frames,
and the **Adler32 of the reconstructed complete RGB555 frame**. Exact
22,050-Hz mono PCM sample alignment after explicit *pre-trim* resampling;
optional public NAR2 lossless compression gives NVA1. Source with no audio
receives deterministic silence.

**Converter and Windows helper actually wired:** opt-in CLI
\`--video-mode nvl1\` / \`nva1\` and \`--video-fps 12\`; GUI has explicit
video codec choice, with old default "skip" to preserve 1.1 compatibility.
Writes \`nds-cache/video/<2-letter-hash>/<full-hash>.nvl\`; original
\`#movie\` instruction unchanged; hashes and full manifests validated;
unconverted movie references still reported. Video conversion never
touches source script, original game, or user saves. Invalid/non-media
inputs fail rather than silently producing an empty video.

**Host evidence:** original 2 s synthetic audiovisual MP4 converted into
both NVL1 and NVA1, frame-by-frame complete *decode back to identical
RGB555* via independent Python LZ4, checksum, packet and audio bounds
audited; silent MP4 and deliberately invalid source exercised. Actual
synthetic PyMO \`#movie\` game converted twice and verified with 0 skipped
movie commands, byte-identical source hashes/script. Full GitHub Actions
[run #39851339676](https://github.com/Blueteemo/CPyMO/actions/runs/39851339676)
**success**, including NTR+TWL dual SDK ROM builds and entire Python
converter suite. No unlicensed FVDS tables or vendor video codec copied.

**Next release blocking gate is the physical movie experience:** built a
separate DSi video review game \`CPYMO12_VIDEO_CHECK\` using the actual
compiled 1.2 ROM and separately converted original SHORT (2 s with
sound), SKIP (8 s with B interruption), and SILENT (1.5 s without source
audio). Normal EOF, skip, error/bad-input behavior, post-playback scene,
BGM reentry, display and touch preservation **must be verified on DSi**
before claiming a finished 1.2 video player. NTR memory and performance
remain open. Mainline 1.1 and known-good R7 unchanged.

## 2026-10-23 — DSi hardware UI acceptance and next release gate

**Owner hardware confirmation:** the custom interactive three-mode DSi demo
played normally and every exercised input/layout scenario passed.
This establishes a *real DSi acceptance baseline for the pre-polish 1.2
UI*, including story, touch choices, X menu and L-mode cycling. Do not
repeat that old claim that enhanced three-state UI is untested.

**Post-acceptance changes actually committed:** follow-up owner-approved
guide text (UP/DOWN, R/L and three named modes, short START/SELECT help)
plus A+B subtle integer RGB555 gradient with an intentionally empty
lower half. The game icon remains at x=196,y=40; help LCD clock at
x=196,y=96; menu clock routing and save-slot action toast retained.
No change to original CPyMO dialogue coordinates or input semantics.
Host test now verifies four-language alignment, LCD reserved x ranges,
the real C renderer, and six cooperative audio ticks per frame:
[UI CI #39847351976](https://github.com/Blueteemo/CPyMO/actions/runs/39847351976)
**success**. NTR + TWL SDK ROM builds and existing converter suite:
[NDS CI #39847457104](https://github.com/Blueteemo/CPyMO/actions/runs/39847457104)
**success** (both jobs). This fine polish **has not itself undergone a
second hardware visual test**; avoid demanding a new device session for
every cosmetic adjustment.

**Next release-critical engineering milestone:** create a legitimate,
reproducible source video → NVL1/NVA1 workflow backed by ordinary FFmpeg,
integrate into converter CLI + Windows helper with bounded output,
independent test media and exact video/audio packet checks; test game's
existing `#movie` EOF/B skip/error/BGM restore with converted assets on
NTR and TWL. This is **not** a demand for further FVDS codec optimisation.
Any real 1.2 release claim remains blocked until converter playback,
source licensing and final regression pass.

## 2026-10-23 — User-approved release-first scope and immediate UI hardware gate

**Priority order has changed by explicit owner instruction:**
1. **3DS-like enhanced dual-screen UI acceptance now.** Hand over a
   fresh DSi/TWL ROM from the present integration branch; visually verify
   scene on physical top, authored textbox/name/text/menu/choices below,
   touch hit-testing, L 0→1→2→0 cycling, Y hide/reveal, return to
   launcher/game, saved mode and responsiveness. Use actual DSi
   observations and optional pixel-exact NDSPIX1 captures; do not keep
   adjusting visual appearance from synthetic previews or unverified
   source-level theories.
2. **Ship real game video support with the converter.** The NDS runtime
   already has experimental NVL1/NVA1 playback and B-skip paths, but
   `cpymo-tool/nds-converter` **currently omits videos by default**.
   A public 1.2 video feature is not accepted until an ordinary user
   can convert their own game's allowed source video via the supplied
   converter, play it from `#movie`, and reach story content again
   after EOF/B skip/error with BGM restored. Prefer an existing
   independently developed reproducible video format; do not make
   unlicensed private FVDS tables a dependency of 1.2.
3. **Consolidated release candidate**: SDK/build + converter tests,
   NTR (4MiB)/TWL (16MiB) hardware regression, source/asset
   licence gate, game save safety, one clearly packaged public 1.2.
   Earlier accepted R7 remains the rollback.
4. **Deferred beyond the release-critical path**: extra FVDS encoding
   size/bitrate/ADPCM optimisation, exotic format sweeps, broad
   profiling unrelated to user-visible breakage, repeated per-commit
   ROM distributions. First make the current features complete.

**This turn's immediate hardware candidate:** targeted Nintendo DS
workflow [#39827066654](https://github.com/Blueteemo/CPyMO/actions/runs/39827066654),
commit `e2b712c37a1f897db8f2d439ce9eb819d50146b6`.
The DSi/TWL ROM is real, has header unit code 2 and passes the ROM SDK
build; the independent converter job may finish later and is not
needed to start **UI-only** acceptance. A single-ROM private review ZIP
includes a short Chinese guide and optional four-scene capture flag,
rather than piling up multiple obsolete candidate ROMs. No new
hardware pass is claimed until the user reports experience.

**Release scope is not conditional on FVDS codec compression progress.**
If the vendor-table license/provenance issue stays unresolved, do not
smuggle the received private decoder into public AGPL source or ROM;
deliver video via legitimate existing NVL1/NVA1 and a supported
converter workflow, or explicitly keep the video milestone open.
Development readiness percentages remain separate from UI hardware
acceptance and may be reweighted after concrete results.

## 1.2 加权进度（开发者估算，非自动百分比）

**当前综合估算：48%（2026-10-22）**。这是面向“可正式发布的 1.2”的**交付准备度估算**，而非完成代码行数、功能个数、GitHub commit 数、工时或自动统计结果。只有新证据通过（主机、SDK 编译或实机）才调整百分比；不随时间自然增长。**不能解释为只剩 52% 工作量**，因为画面逆解码和 ARM7 播放可能存在非线性风险。

| 工作包 | 1.2 权重 | 包内估算 | 加权贡献 | 状态与主要限制 |
| --- | ---: | ---: | ---: | --- |
| 已验证 R7 基础和兼容 | 20% | 95% | 19% | DSi 已验证；还需 NTR 和新分支防回归 |
| 三态双屏与 3DS 风格视觉 | 20% | 65% | 13% | 源码、主机验证与背景预览具备；三态首版 DSi 实机全部通过；后续外观精修待合并验收 |
| FVDS 真正视频画面解码 | 30% | 20% | 6% | 包结构、I/P、bitreader 具备；I 帧 VLC/IDCT/P 运动补偿未完成 |
| FVDS 音频、播放时序与游戏内整合 | 20% | 35% | 7% | 独立 ADPCM/WAV/PCM 环具备；ARM7 混音及完整 #movie 未接入 |
| 版本品质、ROM 集成、许可与发布 | 10% | 30% | 3% | Banner/说明和静态测试；NTR/TWL 已成功构建；视频转换、最终设备回归和发布尚缺 |
| **总计** | **100%** | — | **48%** | **开发估算，不是自动测量** |

**缩短体感空档的交付原则**：将「双屏/交互体验构建」与「FVDS 底层播放器」拆成不同验收关卡。双屏的新三态不必无限等待 FVDS 完成才交给用户：只要 SDK 编译成功并排除明显的崩溃，就准备一次集中的、明确标注“非正式版”的三态视觉测试（含原生 256×192 抓帧）。视频仍由独立研究/实现推进，不得把音频流或 I/P 解析当成画面已解码。

**并行分工**：本地 DeepSeek 专注 FVDS **I-picture VLC/IDCT 独立实现与真实像素对照**，在隔离分支提交 patch；这里负责三态 UI、音频/VRAM 系统接入、合并审查、回归与实机测试包。**不要双方同时改写 `main.c`、`dual-screen-nds.h`、`audio-nds.c`**。未经审查不得改变 R7 或公开 1.2 tag。

## 状态计分和同步纪律

每次回答用户时报告 **48% 或按真实里程碑更新后的新分数**，附最新通过的门槛、下一待突破门槛、是否产生了新的可测试构建。来源文档：本文件。开发中的小修补不自动加分；后续依照相同权重和标准复估，并注明主观性。没有设置自动实时监控，不可暗示数字每秒自动刷新。

## 2026-10-22 — 本地 DeepSeek 并行交接及三态体验构建

- **当前综合进度仍为 48%**。新一次编译通过使近期双屏视觉测试具备了实际 ROM，不等于新版三态已实机验收或 FVDS 真正播放，暂不为小成果加分。
- 完整源码离线开发快照来自 **`d5be738231570ba16071507a3aa01ffe4b6ba276`**，GitHub Actions [开发交接快照](https://github.com/Blueteemo/CPyMO/actions/runs/39703898561) 通过。交接 ZIP 包私有地组合了原 159s j1 FVDS、1s/3s 合成截取样片、既有研究证据和 DeepSeek 提示词，**私有游戏媒体未推送至 GitHub**。
- 本地 DeepSeek **单独负责第一张 I 帧独立 VLC/IDCT 解码**，不得修改 `main.c`、`dual-screen-nds.h` 或 `audio-nds.c`，不得未经许可复用 FastVideoDSPlayer。成果以隔离 patch + 自动测试 + 真实像素/失败证据返回；云端主线负责审查、三态 UI、音频/VRAM、游戏 `#movie` 接口。
- **一次完整 NDS 和 DSi SDK 编译、所有转换器测试均通过**：[Nintendo DS #39704544801](https://github.com/Blueteemo/CPyMO/actions/runs/39704544801)。这次 devkitPro 服务可用；构建输出为 `engine.nds` 和 `cpymo-dsi-experiment.nds`。后者已经打包为“DSi 三态双屏视觉体验版”，保留独立文件名和 R7 回退，**明确尚未上机验证**。
- **不再绑定视频解码完成才测试三态双屏。** 已经有可选的三态体验包，用户可在方便时进行一次集中视觉/触控验收，并提供 `NDSPIX1` 实际画面；FVDS 由本地 DeepSeek 并行研究，直到真正输出 RGB555 画面才宣布视频进入显示阶段。
- 成功全构建 **只证明可编译**，不证明真实 3DS 风格上下屏在每款游戏都无错、正常 DS/NTR 性能达标或 FVDS 视频播放成功。仍不发布 1.2.0。

## 总目标和证据分层

1. **R7 不倒退**：DSi/TWL 16 MiB、DS/NTR 4 MiB 两种环境及启动器；游戏库、滚动和触控；DSpico TF → DSi 机身 SD 游戏跨卡读取；资源缓存、正常原版 PyMO 脚本；旧版 NVL1/NVA1 视频、B 跳过和 BGM 恢复。
2. **经典＋3DS 风格增强双屏**：独立保留经典上屏、经典下屏及增强双屏三个形态；L 单键循环，上屏场景＋下屏对白／选项／菜单；触控以物理下屏实际内容为准；不改变原作的姓名框、对白框美术。
3. **FVDS 真正播放**：以 256×192、12fps、47,605Hz 立体声 FVDS j1 159秒/1908帧作为验收素材；包括 I/P 画面解码、播放音频、资源所有权、完整播放、B跳过、异常和 EOF 返回剧情。
4. **可正式公开的质量**：正确 Banner 署名、版本号、四语言界面、NTR/TWL 内存预算、ROM 构建与回归；第三方播放器/编码器尚无明确授权，不导入其源码。

**验证等级不要混淆：**
- **实机通过**：用户此前 DSi/TWL 对 R7、早期两屏视觉基础及原生 NVL1/NVA1 旧格式 B 跳过的验证；不等于新版三态/视频已合格。
- **主机通过**：实际 C 生产模块用 GCC/UBSan、用户私有样片经过电脑验证；不等于 NDS ROM 能运行。
- **已实现待验证**：1.2 新双屏渲染、模式恢复、PNG 真实屏幕抓取和 Banner Unicode 补写等；需完整 ARM9/ARM7 ROM 构建以及 DSi 屏幕检查。
- **未实现**：FVDS VLC 量化系数完全解码、图像逆变换/运动补偿/RGB555 真正帧解码、ARM7 音频输出集成、完整 `#movie` 游戏内生命周期。

## 工作流状态矩阵

| 模块 | 1.2 当前状态 | 已经有的证据 | 下一门槛 |
| --- | --- | --- | --- |
| R7 稳定基础（游戏库、跨卡、缓存、旧视频） | 实机通过的回退基线 | 用户的 DSpico/DSi 测试；NVL1 B跳过 | 新分支全量回归，普通 NTR/4MiB 也需检查 |
| 三态 L 切换/每游戏记忆 | 实装，主机部分测试 | `test_display_mode_prefs.py`；源代码切换逻辑 | ARM 构建 + 三种形态触控和重新进入游戏 |
| 3DS 风格下屏对白、姓名框、菜单与选项 | 实装，尚未视觉定稿 | 原版 3DS draw-type 对照；`test_nds_dual_layout.py`；源码 RGB555 底层预览 | 真正的上下屏游戏截图，菜单位置和遮挡验收 |
| Banner Unicode 三行信息／NDS移植署名 | 实装，主机通过 | `test_nds_rom_banner.py`；保留 `千岚之夏` 且 1.1.0 未冒充 1.2 | 真实烧录卡菜单的 3 行显示 |
| UI 四语言与按键说明 | 代码存在，待 1.2 综合回归 | 语言资源及启动器帮助 | DSi 模式＋NTR 模式中的切换体验 |
| FVDS 转换器预检和原始包边界 | 主机通过，早期只读预检在 DSi 验证 | `fvds_inspect.py`、`fvds-stream-nds.c`、七项实机包早期结果 | 与真正解码播放在 `#movie` 合流 |
| FVDS ADPCM 立体声块解码 | 生产 C + 真实159秒主机通过 | `fvds-adpcm-nds.c`、`fvds-to-wav.c`；完整PCM长度/数值校验 | 适配现有 Maxmod/Calico ARM7 音频 |
| PCM 双帧预读与采样时钟 | 主机模型通过 | `fvds-pcm-queue-nds.c`；47,605 Hz 理论缓冲预算 | 真实硬件的运行延迟、同步和内存峰值 |
| FVDS 图片容器/I-P头/位流入口 | 主机真实素材通过 | `fvds-bits-nds.c`、`fvds-picture-audit.c`，本次私有 j1 完整检查 | 实现并比对首张 I 帧的真正像素 |
| FVDS VLC/DCT/运动补偿 | **未实现** | 无已完成可验证的 RGB555 帧 | 完整解出 I，再完成 P，量测 NTR 性能 |
| 电影音画/显存/EOF及异常恢复 | **未接入 FVDS 正式播放** | 旧 NVL1 的 B 跳过通过；FVDS stage 只有主机回调测试 | 完整 `#movie` 播放和三态界面恢复 |
| ROM ARM9/ARM7 构建 | 最新提交**未验收** | 老集成版本曾构建成功；devkitPro 403 偶发打断更新 | SDK 可用时集中编译，不为日常提交频繁触发通知 |
| 稳定分发/文档/许可 | 未满足正式发布门槛 | 1.1.0 保留，外部 FastVideoDS 授权申请仍待解决 | 1.2 正式构建、兼容、归档和版本更新 |

## 本轮增量：真实 FVDS I/P 头部全片核验

新建独立 C 工具 `cpymo-backends/nds/tools/fvds-picture-audit.c`，以 **CPyMO 生产版 `fvds-stream-nds.c` 和 `fvds-bits-nds.c`** 为输入，不使用第三方播放器解码代码。它逐帧读取首个字、I/P 类型及 I 帧六位量化参数，和完整索引对应，不会越界读入音频。通过合成样例正/负测试及 [CI run 39702549940](https://github.com/Blueteemo/CPyMO/actions/runs/39702549940)。

用同一个 Actions 产物在本地读取用户原始私有 `full-j1.fv`，得到：
- 1908 帧；8 个 I，1900 个 P，均与关键帧索引一致；I 帧序号 **0, 251, 502, 753, 1004, 1255, 1506, 1757**。
- **8 帧的量化参数均为 30**。视频压缩载荷 **5,129,324 字节**，立体声 ADPCM 载荷 **7,805,688 字节**；全文件 12,942,736 字节。
- 原私有 FVDS、WAV、原始游戏媒体和完整图像 **没有**提交到公开仓库；仅公开可复现的程序、合成测试与数值。
- 这仍只是**可依赖的 VLC 解码入口**。没有完成首张 I 帧实际逆变换、像素重建，不能展示虚构截图或宣称视频已播放。

## 下一轮优先任务（非一次只做小修补）

**工程优先顺序**：
1. 自行设计／验证 FVDS I-picture VLC 系数解码（有界、错误可恢复，逐 4×4 块）；然后完成量化恢复、逆变换、帧像素核对，生成真实的 256×192 I 帧。绝不把仅解析头部当作解码成功。
2. 完成 P-picture 运动补偿与帧引用；测量在普通 NTR 4 MiB 中的峰值内存与每帧耗时。
3. 与 ARM7/Maxmod 协同，采用已验证的双帧预读和 bounded PCM 环，统一 EOF / B 跳过 / 读取错误 / 合盖后退出的资源释放，恢复剧情、BGM 和三态 LCD 映射。
4. 集中验收 3DS 视觉方案，使用**真实** NDSPIX1 上下屏像素截图，不使用生图；经用户认可后再作为 1.2 定稿。
5. 发布候选版时跑包括普通 DS/NTR 与 DSi/TWL 的完整测试，并把 Banner 版本更新为真正发布的 **1.2.0**，而非现在提前冒充。

**预计实机轮次目标（非保证）：** ① 三态双屏视觉／触控；② 完整 FVDS 视频音频与跳过；③ 视前两轮结果决定的最终候选版综合回归。中途只在出现硬件特有问题时额外加专项回归。下一轮不需要用户现在测试。

## 汇报约定

从此所有 1.2 开发回复结尾统一更新以上五个层面：① R7 与稳定功能是否受影响；② 3DS 双屏视觉和触控状态；③ FVDS 视频画面／音频各自状态；④ 构建与质量（实机 vs 主机）；⑤ 最近真正推进什么、下一步卡在哪、需要几轮上机。**不用凭主观感受虚构总体完成百分比或交付日期**。


## 本地 DeepSeek 单次长任务专项（2026-10-22）

用户要求减少协作过程中的碎片化确认、增强单轮交付深度。因此本地 DeepSeek 的原任务「仅解第一张 I 帧」升级为 **独立 FVDS I/P 全片视频解码专项**：在本地一次迭代中研究 VLC 语法、逆量化/逆 DCT、I/P 参考和运动补偿、159 秒/1908 帧 RGB555 离线输出、原 MP4 独立质量比对、内存与性能测试、可复核补丁。**首张 I 帧现在只是任务内部里程碑，不是交付终点**。

新私有交接包 `CPyMO_1.2_DeepSeek_FVDS全片解码_长期并行开发包_20261022.zip`（约 34 MB）采用 GitHub 固定源码快照 `d5be738231570ba16071507a3aa01ffe4b6ba276`，另含原始用户 FVDS 三样片、`source_n2_opening.mp4` 与必要研究证据。原视频/MP4 均**不**上传 GitHub。交付物是本地 `DeepSeek_FVDS_FullDecoder_Review.zip`，包括 `PATCH.diff`、测试、真实像素及质量验证、性能和内存报告；未解格式必须给出可复现实验和诚实 blocker，不能冒充全片解码。

同时坚持**互不覆盖**：DeepSeek 仅改独立视频 decoder/CLI/tests，不动 `main.c`、`dual-screen-nds.h`、`audio-nds.c`、ROM Banner/版本号/存档，不直接向仓库 push；云端主线继续完成三态双屏视觉/触控和 ARM7/显存/游戏 `#movie` 接入。原播放器授权不明，禁止移植/复制未授权源码。解码结果只有被独立复核并合入后才计入正式进度。

**综合交付准备度仍为 48%**：委派任务与生成开发包本身不增加实际完成度。新版三态双屏 DSi 实验 ROM 在 [完整 CI #39704544801](https://github.com/Blueteemo/CPyMO/actions/runs/39704544801) 构建通过且可选实机体验，**并不需要等待 FVDS 全片解码**。后续只有证据门槛突破才更新百分比。


## 2026-10-22 10:28+08 — 云端并行：三态双屏触控修复、状态机回归

本地 DeepSeek 已启动其**完整 FVDS 视频画面解码专项**，当前本集成分支**不**开发或修改独立视频 VLC / I/P / IDCT 解码器，以免重复劳动和代码冲突。本轮集中改进用户能够更早在 DSi 上感受到的三态交互：

- 新增 `source/dual-input-nds.h`，并在实际 `source/main.c` 的 `cpymo_input_snapshot()` 中调用。使用**物理下屏**为触控基准：经典上屏以触控作为 A；经典下屏按游戏真实坐标点击；增强双屏菜单／剧情选项转游戏逻辑坐标以供核心热区判断，叙述状态触控作为 A；Y 隐藏后的唤醒触控另由现有遮罩阻止穿透。不同原作分辨率及 letterbox 偏移统一计算。
- 修复 `source/dual-screen-nds.h` 的 **L 三态切换＋仍按住触控笔**：标记 `nds_touch_blocked` 并调用 `cpymo_input_ignore_next_mouse_button_event()`，直到物理抬笔，不允许旧模式的触控释放在新模式中误选菜单/选项。保持普通 A 键可用，不动旧版 R7。
- 新增 `test_nds_dual_touch.py` 与 `test_nds_display_machine.py`：前者直接 GCC/UBSan 编译生产触控映射，覆盖三态、菜单、选项、唤醒、按键、letterbox；后者带有限宿主桩编译**真实** `dual-screen-nds.h`，循环三个模式并检查 `NDSDISPLAY2` 实际存档、上下屏类别归属及触控锁定。通过 [独立 NDS UI 主机 CI #39712825259](https://github.com/Blueteemo/CPyMO/actions/runs/39712825259)。
- 本轮**没有修改 RGB555 下屏底色、原作对白框、姓名框及画面布局**，因此没有全新游戏视觉截图或合法的画质对比图。过去的源码底层效果仍为有效参考，真实整体画面仍要以新版 .nds 实机 NDSPIX1 抓帧验收。
- **48% 综合准备度暂不提高**：按权重计分，这轮属于三态稳定性与验收准备，尚未增加 FVDS 可用画面或 DSi 新版实机通过记录。R7 稳定分支与原版 MV 播放逻辑没有改动。新三态测试 ROM 可在合并 CI 验证后提供，且不要求用户立刻上机。



### 10:45+08 SDK 构建与触控体验版验收门槛

- [NDS SDK 全构建 #39713204905](https://github.com/Blueteemo/CPyMO/actions/runs/39713204905) **成功**：原 NDS 与 DSi/TWL 两份 .nds 均通过构建，转换器回归测试通过；这验证本轮触控改动可进入 ARM9/ARM7 ROM 工程，但尚未由用户在 DSpico/DSi 上体验。
- 发现并修复开发效率根因：固定 Docker 镜像 `devkitpro/devkitarm:20260610` **已经预装 devkitARM、libnds 2.0.2、libfat-nds 2.1.0、maxmod-nds 2.1.0、calico 1.2.0、ndstool 2.3.1**。此前每轮即便包已完整也强制 `dkp-pacman -Syu`，导致软件源 HTTP 403 阻断整个构建。在 `.github/workflows/nds.yml` 改为**先检查镜像中已安装软件包；只有缺包才联网安装**。这次验证未触发无谓的软件源同步且真实 SDK 构建成功。维持 PR CI 与人工里程碑触发策略，不对每次代码 push 轰炸通知。
- 制作了仅含 **一个新 DSi 三态体验 ROM + 使用说明** 的 `CPyMO_1.2_DSi_三态触控修正版_20261022.zip`（基于成功的完整 SDK 构建提交 `7678691828cc363afa54a627e4ddd0f339b7dab0`），只供未来方便时集中上机验收。此前经过验收的 R7 原版及存档不覆盖，不把 1.2 非正式体验 ROM 多版堆进 APPS。
- **综合交付准备度仍 48%**。本轮对用户体验和构建可靠性有实质改进，但新三态界面未经过实机验证，也没有来自本地 DeepSeek 的 FVDS 解码合格成果，按原计分体系暂不增加进度数字。
- **预计剩余 2～3 轮集中上机（目标非承诺）**：① 已具备新版三态触控／视觉可选体验 ROM，② 等 FVDS 全帧播放再测音画及 B 跳过，③ 按需要综合候选版与 NTR 4 MiB 回归。现阶段不催用户立刻复制或测试。

## 2026-10-22 — physical LCD evidence coverage expansion (this session)

- The integration branch now expands the strictly opt-in `NDSPIX1` capture
  contract from 2 to 4 one-time events per game: **enhanced dialogue,
  enhanced UI/menu, enhanced script choices, and classic-lower dialogue**.
  The fourth event records the swapped physical upper auxiliary framebuffer
  first, then the main game framebuffer physically displayed below.
- Native 256x192 RGB555 output is unchanged, as are the actual scene/layout
  renderers, UI widgets, key bindings, game resources and saved modes.
  The first choices capture takes precedence over a lingering dialogue
  state, so comparing interactive sprites to touch targets is possible.
- Added C writer / Python decoder test coverage for capture kinds 3 and 4,
  including exactly one write per kind, physical screen order, and malformed
  kind rejection. Tests use **synthetic pixels only**, not screenshots.
- This work directly prepares the first concentrated three-mode hardware
  review, but is **not a new visual acceptance nor a new FVDS decoder**.
  Keep weighted delivery readiness at **48%**, pending host CI, ROM SDK
  compilation and genuine DSi NDSPIX1 captures.

### 四场景诊断验收结果与实验 ROM（2026-10-22）

- 主机 NDS UI 回归 [#39715235344](https://github.com/Blueteemo/CPyMO/actions/runs/39715235344) **成功**，覆盖生产 C NDSPIX1 writer、Python PNG converter、三态显示与触控、layout、banner 等原有测试。
- 精确代码提交 `55aa190a5cc81c6dfc2d070c35eb6f8d1e117d14` 的 [Nintendo DS #39715423370](https://github.com/Blueteemo/CPyMO/actions/runs/39715423370) **build 与 converter 均成功**；生成普通 NDS 与 DSi/TWL 实验 ROM。
- 已检查 DSi ROM：820,224 B，SHA-256 `8df84303df19aa1ba3808eaf87eb80738bcd0cf89219bd8cbf536945b2c61e66`；Banner 六个语言槽含 `CPyMO for NDS / 1.1.0 / 千岚之夏`，NDS unit code=2，保持实验构建的真实版本。
- 单 ROM 自包含体验包 `CPyMO_1.2_DSi_三态四场景体验版_20261022.zip` 包括 DSi 构建、中文验收说明和 SHA-256 校验。它是可选测试资产，不覆盖 R7、不正式发布、不意味着 DSi 实机已通过。
- 上机时应优先检查 **L 三态/触控精确选项、上下屏真实画面、X 菜单、Y 隐藏、切游戏恢复**，可用四个 `nds-capture-*.ndsraw` 一次性交付实际像素。待用户方便时集中验证，不要求立即测试。
- **综合准备度仍为 48%**，新增回归和可用测试构建不等于 FVDS 解码成果或实机视觉验收。

## 2026-10-22 — 三态体验源代码审计与核心输入/反馈修复（本会话）

- 把优先级从“更多可视化测试设施”调回实际游玩：确认上一轮四类 NDSPIX1 抓帧仅用于可选硬件验收，现阶段不再扩建诊断工具。
- **修复 L 换屏时虚拟 A 的释放误触**：老实现在用户触摸经典上屏辅助屏并按 L 时，仅调用 `cpymo_input_ignore_next_mouse_button_event`；而该触摸经 `nds_touch_project` 被转换为 `input.ok`，并非 `mouse_button`，下一次核心更新可能接收到非预期 `ok` 释放边缘。现在模式循环前清除旧触摸的 `ok` 与 `mouse_button`、无效坐标；保留当前真实 `KEY_A`。已有触摸锁仍保持到物理抬笔。直接使用生产 `dual-screen-nds.h` 的宿主状态机回归覆盖该复现路径。
- **修复增强双屏下快速存档/读档反馈不可见**：以前 `launcher_toast` 只重绘经典辅助屏，而增强模式禁止重绘辅助屏，导致成功/失败提示在增强模式看不到。现在增强模式使用现有画布，在上屏短暂绘制文案；2 秒后请求刷新，避免剧情静止时残留。下屏对白/选项仍保持独立，经典模式保留原辅助面板提示。代码未触碰 FVDS 视频解码器。
- 实际证据：输入/视觉主机回归 [NDS UI Host #39717449670](https://github.com/Blueteemo/CPyMO/actions/runs/39717449670) **通过**；包含上述两处修复的 [Nintendo DS SDK + converter #39717849111](https://github.com/Blueteemo/CPyMO/actions/runs/39717849111) **build / converter 双作业均通过**，提交 `fc1c02cff267a22f3b125bf349b5b4c21f093efa`。产出了 NDS 和 DSi 实验 ROM；尚未经过 DSi 新版硬件触控/画面反馈验收。
- **总体开发者交付准备度仍记 48%**，不把编译成功当作实机验收；R7 稳定分支与公开 1.1.0 保持不变。下一门槛是集中实机确认切换触控、菜单/剧情选项位置、对白与短暂提示；FVDS 全片解码继续由本地 DeepSeek 独立开发，待提交审查。

## 2026-10-22 — Held stylus: enhanced narrative ↔ choices/menu role boundary

This session fixed a *different* ghost touch from the previously completed L-mode
switch: the SAME uninterrupted lower-LCD pen contact can span the moment an
enhanced narrative (`touch → OK`) changes into engine UI or scripted choices
(`touch → mouse coordinates`) or vice versa. Without handling this boundary,
the old contact can be interpreted as a new selection/click or a synthetic
A-release in the next CPyMO input update.

- Added production portable `nds_touch_role_tracker` and
  `nds_touch_role_step` in `source/dual-input-nds.h`. It detects whether a
  previously held pen crosses between narrative and coordinate-hit-test roles;
  fresh touches arriving after a UI transition are left untouched.
- Integrated `nds_touch_prepare_input` into `source/main.c` **before**
  `cpymo_engine_update` copies `engine.input` to `prev_input`. At a real
  contact boundary it neutralizes the OLD pen-derived OK/mouse fields, preserves
  the actual physical A state and blocks a continuing pen until release.
  Existing independent L mode-switch and Y auxiliary wake guard remains.
- Added real-header GCC/UBSan regression sequences for story → choice/menu,
  mouse → narrative with release, continuous vs fresh contact, no-contact
  transitions and invariant classic-top / classic-bottom routing.
- The first focused host run #39718806455 failed at compile-time in a new
  test fixture (`NULL` used without `<stddef.h>`); this was fixed and the
  repeated host gate [#39718909521](https://github.com/Blueteemo/CPyMO/actions/runs/39718909521)
  **passed**.
- The concentrated [NDS/DSi SDK + converter run #39718962687](https://github.com/Blueteemo/CPyMO/actions/runs/39718962687)
  **passed both build and converter** on commit
  `a1b7f972d3cdef4a703b83e0958e7fbf1d585186` and produced both ROM
  artifacts. These are source/compiler test results, **not real DSi/NTR
  acceptance**. No public v1.2 release; R7 remains unchanged and the
  parallel DeepSeek FVDS decoding work is untouched.
- Overall weighted deliverable readiness remains **48%** until actual
  enhanced UI acceptance and real decoded FVDS playback integration occur.

## 2026-10-22 — Three-mode L-switch responsiveness and mode-aware classic helper

Unlike the earlier touch correctness fixes, this milestone addresses an
unnecessary synchronous FAT write **on every L screen-mode switch**.
Previously `nds_display_cycle()` called `nds_dual_save()` immediately.
Fast 0→1→2→0 cycling therefore opened/wrote/closed the same game save file
three times, risking visibly delayed LCD switching on slow flashcarts.

- `dual-screen-nds.h` now records the selected mode immediately in memory,
  redraws the chosen display without saving synchronously, and schedules a
  single per-game preference write after **750 ms since the LAST L press**.
- `main.c` calls `nds_dual_poll_mode()` from the existing frame loop. On
  gameplay exit `nds_display_restore_launcher()` flushes pending state
  **before the active engine frees its game-directory string**, preserving a
  last-second choice; each game starts with a cleared pending flag.
- The portable host state-machine regression exercises real emitted bytes,
  premature poll/no file, repeated L postponement, one final mode-2 save,
  subsequent persisted mode-0 save, one coalesced two-switch sequence,
  immediate exit flush and reload of its chosen game mode. No game save
  slots, RAM scene cache, R7 code or FVDS codec files were altered.
- The classic auxiliary panel displays mode **1/3** or **2/3**, and classic
  LOWER uses a distinct "A continue / touch to control the game" hint
  rather than implying that its physical lower-screen coordinate input is
  always the virtual-A helper. The new text has English, simplified/traditional
  Chinese and Japanese translations; enhanced mode retains its authored
  lower-screen dialogue/menu canvas.
- Evidence: [UI Host Regression #39720443566](https://github.com/Blueteemo/CPyMO/actions/runs/39720443566)
  **passed**. [Nintendo DS full SDK + converter #39720506030](https://github.com/Blueteemo/CPyMO/actions/runs/39720506030)
  **passed both original NDS and DSi/TWL builds and converter regression**,
  exact tested commit `5c7a6c59fa56b2959e17e662bc4cdad2c952d79d`.
- Scope: this removes *redundant writes*, not the single actual persistence
  write; FAT timing and the enhanced visual result still require optional
  hardware acceptance. Existing R3 is an earlier ROM and does **not** contain
  this change. No R4 test claimed and no 1.2 release made.
- Weighted 1.2 delivery readiness stays **48%**, since FVDS real video
  reconstruction/output and the final combined NTR/DSi device visual tests
  are still open, independently of these successful UI changes.

## 2026-10-22 — Local DeepSeek full FVDS decoder handoff, online independent audit

Received the private `DeepSeek_FVDS_FullDecoder_Review.zip` (59 MiB
compressed, approximately 209 MiB expanded). **No source, film frames,
VLC tables or private licensed media were pushed into GitHub.**

- Delivered: an original-host C VLC + inverse-dequantisation/4x4-IDCT
  + I/P/motion-compensated picture decoder, `PATCH.diff` with eight new
  source/test files, 1,908 decoded 256x192 RGB555 frames and PNGs,
  per-frame SHA/CRC CSVs, quality report, source-level unit tests.
- Verified here: all 1,908 RGB555 SHA-256 hashes **match** their delivered
  manifest; 1,814 distinct pixel hashes, 92 **consecutive** still-frame
  repeats, eight indexed I pictures, 1,900 P pictures. Sampled PNG pixels
  exactly equal each frame's 5-bit RGB channels expanded by `<<3`.
  Delivered CRCs refer to **G/R/B planar bytes**, not packed RGB555;
  independently reconstructed and verified several. `crc.csv` is identical
  to `san.csv` (internal equality, not independently rerun sanitizer).
- Built the supplied C video dump tool with Linux GCC
  `-std=c11 -O2 -Wall -Wextra -Werror`, and ran the supplied
  coefficient/IDCT/motion test against actual source C successfully.
  Eight patch paths re-applied successfully; five byte-level diffs from
  the included flat sources are CRLF vs LF **only**.
  Packaged baseline `fvds-bits-nds.c`, `fvds-stream-nds.c`,
  `fvds-picture-source-nds.h` exactly match the current integration
  branch, so rebase-interface risk is low. This is an offline host audit,
  **not** an independent full replay from the copyrighted source FVDS
  (source video was not in the review ZIP).
- Found a **production hardening issue before ROM integration**:
  `nds_fvds_vlc_fill` synthesizes zero bits at payload EOF without a
  strict bound. A standalone 2-byte all-zero vector returns success
  after 64 logical bits and 75 synthetic zero bits; the shipped unit
  test explicitly allows this. Need distinguish legal final-word padding
  from a corrupted/truncated picture and add malformed-stream tests.
- The I-picture quantiser is parsed in the dump path but not passed to
  `nds_fvds_decode_picture`; the actual shipped sample uses q=30.
  Confirm support/validation for other legal quantiser values rather
  than implying the decoder is generally format-complete.
- `perf.csv` is byte-identical to `crc.csv` and has no durations, so
  the reported ~0.705ms desktop mean speed cannot be recalculated
  from this bundle. A real ARM9 12fps/NTR 4MiB test remains open.
  Decoder local-memory estimate (~140KiB for two planes + tables + stack)
  is not a whole-application peak RSS guarantee.
- Third-party provenance remains to be checked before moving
  generated VLC/bias table content into a **public AGPL repository**:
  the original vendor encoder source is described as unlicensed.
  Do not assume a clean-room claim automatically clears table reuse.
- **Engineering next:** harden decoder boundary and quantiser tests;
  stage the standalone decoder privately; implement cooperative video
  packet/PCM dispatch and a reusable graphics presentation lease;
  validate normal EOF/B skip/errors/lid/audio resume and the physical
  12fps budget on both NTR 4MiB and TWL 16MiB before public 1.2.
  No 1.2 playable FVDS ROM or hardware acceptance is claimed yet.

## 2026-10-23 — FVDS private integration M1, owned entirely by this project

The user asked the online CPyMO team to continue the implementation without
outsourcing later steps to local DeepSeek. The received full-decoder work has
been staged **privately** and treated as input, not as a merged ROM feature.

- Built and tested a concrete `fvds-playback-bridge-nds.[ch]` joining the
  existing FVDS stream reader to the provided I/P picture decoder. Its per-frame
  callback contract carries a rational microsecond PTS and the decoded planar
  G/R/B bytes; it decodes existing 264-byte FVDS ADPCM pairs via the already
  public `nds_fvds_adpcm_pair` into 256-frame interleaved stereo PCM blocks.
  This is a host-side operational callback boundary, **not ARM7 sound playback**.
- Verified an entirely synthetic 3-picture **I→P→I** FVDS container through
  the real C packet reader, VLC, IDCT, motion/reference chain, ADPCM decoder
  and callback bridge. Regressions also cover natural EOF, early user skip,
  unsupported q (reject q!=30 rather than misdecode), a truncated I payload,
  malformed ADPCM index, pixel-channel packing, and preservation of packet
  audio/video boundaries. Compiler uses `-Wall -Wextra -Werror`, the bridge
  run uses AddressSanitizer/UBSan, and both new bridge and the adjusted native
  coefficient tests pass locally.
- Tightened the *private* VLC coefficient decode: consuming more than the
  physical picture payload plus 15 legitimate terminal pad bits now yields a
  truncated error, preventing the observed two-byte synthetic zero-fill
  false success. This is not yet independently checked against the original
  full 1908-frame FVDS source, which was not present in the decoder review ZIP.
- Corrected a material decoder memory accounting error in the handoff:
  `2 * 3 * 256 * 192 = 294,912 bytes` (288 KiB) for two triple-plane
  reference frames, NOT 100,368 bytes. VLC lookup constants separately use
  about 35 KiB. Allow for audio queue, original engine and runtime stack on
  NTR 4 MiB; no assertion of completed NTR allocation acceptance yet.
- The supplied host `.rgb555` files encode red in the **high** five bits.
  Actual libnds `RGB15(r,g,b)` places red in the **low** five bits and blue
  in the high five. A direct planar → existing CPyMO framebuffer conversion
  (with bit15 set) is verified; there is no third full-frame RGB555 buffer.
- The private handoff `CPyMO_1.2_FVDS_私有集成M1_源码与回归_20261023.zip`
  contains the self-contained C prototype, synthetic unit tests and delta.
  No copyrighted movie content and no new decoder VLC-derived tables were
  uploaded to GitHub. Original stable R7 and 1.1.0 remain unchanged.
- The FastVideoDS Player/Encoder public repositories have no clear root
  code-merging licence; upstream license-request issue #15 was still open
  during this review. Before pushing vendor-derived table data into public
  AGPL code, provenance/permission must be resolved.
- **Next milestone (M2):** schedule the decoded PCM using the existing
  pre-roll queue and rational 12fps clock, implement safe ARM9 display and
  VRAM lease plus ROM-local `#movie` transition, with one cleanup path for B,
  EOF, errors, lid, BGM and script continuation; then benchmark actual
  NTR and DSi hardware. This M1 is deliberately not called a video-playing
  `.nds` and does not increase the accepted 48% integrated release readiness.

## 2026-10-23 — Public GitHub 1.2 release protection (required)

After user requested that FVDS integration must **remain publicly publishable**,
audited the private DeepSeek handoff generator. Its
`tests/gen_vlc_tables.py` reads upstream
`Gericom.FastVideoDS/VlcTables.cs` (`TableA`, `TableB`,
`TableARefLinear`) and emits the decoder's ~84 KiB VLC lookup header.
The vendor Player/Encoder repositories have **no declared licence**
(GitHub API returned `license:null` as of 2026-10-23), and licence
request [#15](https://github.com/Gericom/FastVideoDSPlayer/issues/15)
was **open**. Therefore the private full decoder + M1 bridge is
**not yet suitable for redistribution inside AGPL-3.0 source or ROMs**.
"Independently written" C glue does not cure vendor-data provenance.

Created mandatory
[`RELEASE-LICENSE-GATE-1.2.md`](RELEASE-LICENSE-GATE-1.2.md)
and an opt-in-host/full-converter
`test_nds_fvds_release_gate.py` that blocks known quarantined
received source files, VLC signature constants and named private media.
It is a safeguard, not complete legal verification and not a blanket
ban on FVDS as a format. The targeted
[NDS UI host workflow #39826200756](https://github.com/Blueteemo/CPyMO/actions/runs/39826200756)
passed with the gate enabled; the regular NDS converter workflow
discovers the same test on its next requested build.

Future open-release path: obtain an explicit AGPL-compatible
source/lookup-table grant **or** independently replace the affected
codec internals with documented clean provenance and review. Work on
generic A/V scheduling/PCM/VRAM/EOF/B skip may continue without those
tables. Existing independently developed NVL1/NVA1 remains an
optional releasable fallback if codec permissions cannot be resolved.
Do not tag or advertise public FVDS playback using an unreviewed
decoder. R7/1.1.0 untouched; overall accepted v1.2 readiness still 48%.

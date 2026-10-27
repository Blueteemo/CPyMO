# CPyMO for NDS 1.2.0 — 发行工程与最后验收

## 桌面转换器正式应用图标（2026-10-27）

- [x] 视觉方案：采用 10 月 25 日资料库的 **04_高清像素图标_透明外框.png**，而非 02 黑底版本。原件尺寸 1254×1254 RGBA，SHA-256 `3f110860ce4e242b79147ccd7b6c6d260efd1a232fa8039b41c02677aa3f8c62`。
- [x] 已在本地导出并校验 1024px PNG、64px GUI PNG、16–256px Windows ICO、支持 Retina 的 macOS ICNS；交付完整 04 原件与图标包。
- [x] Github 开发分支已加入确定性图标生成工具，Windows PyInstaller `--icon` / Tk、macOS ARM64 `--icon` / `CFBundleIconFile` 的接入及防默认图标校验；UI 回归测试已通过 [40533116706](https://github.com/Blueteemo/CPyMO/actions/runs/40533116706)。
- [ ] **图标源 PNG 二进制尚未写入 GitHub。** 从提供的 `CPyMO_1.2_桌面转换器04号图标_完整交付包.zip` 中，将 `GitHub-ready/cpymo-tool/nds-converter/assets/converter-icon-source.png` 按原字节放入对应 repo 路径。上传后必须核对 SHA-256 与上述 digest；不拿低分辨率备选代替。
- [ ] 重新运行两端构建并传入 `--require-desktop-icon`，复核 Windows `.exe` PE icon / 任务栏以及 macOS `.app` 的 Dock/Finder 图标、Info.plist 与源 ICNS 一致；软件逻辑和 NDS ROM 不做重测。
- [ ] 正式公开附带用户选定的 PyMO 衍生视觉前，核对上游图案的使用/转载许可或取得必要授权。图像制作不意味着自动享有原版权。


此清单仅用于发版操作，优先级高于早期 1.2 研究与阶段性试验文档。
用户已完成 R4 Wood DS/NTR 和 DSpico DSi/TWL 的本地 ROM 实机验收；
不要再要求用户重复进行已经验收过的 NDS 游戏端测试。

## 必须完成的步骤

- [x] 生成且校验通用 `cpymo-nds.nds` 的 1.2 候选：NTR/TWL 使用同一 ROM，02-B 星夜背景 SHA-256 与实机验收素材一致，未保留 placeholder；后续冻结发布提交时须再次核对输出哈希。对应 CI：https://github.com/Blueteemo/CPyMO/actions/runs/40490746670
- [x] Windows Starter **RC** 嵌入相同 SHA-256 的通用 ROM；默认 NVL1、缺字自动方框、输入源完整性及实际 FFmpeg 视频转换的冻结工作进程测试通过。正式发行提交冻结后须重建并重复核验。
- [x] macOS Intel `.app` **RC** 完整打包并通过 CI 视频转换及配套 ROM 校验；附 FFmpeg 7.1.5 完整对应源码与 LGPL 编译参数。**尚未通过用户自己的 Finder/Apple Silicon 图形验收**。
- [x] 已在本轮发版会话向用户提供**经过核验的 macOS LGPL RC ZIP**（SHA-256 `d99ca80e64ecdb098cb4e116f4eea7625109dbb9869c597e70dd6f4176168546`）及可直接交给 DeepSeek 的独立中文验收提示词；明确要求验证 Finder、ZIP/APK/目录、默认 NVL1/NVA1、缺字、源文件与存档保护、Apple Silicon/Rosetta，并禁止关闭 SIP 或 Gatekeeper。
- [ ] 收到 DeepSeek 的 Mac 实测回报并复核。未通过或有 blocker 时修正候选后再决策；不将内部 macOS CI 冒充用户本机 Finder 验收。若最终无法满足 macOS 发行门槛，Mac 版本可延后，但须明示。
- [ ] Windows/macOS 包、源码及许可证梳理、FFmpeg 构建信息、SHA256SUMS 与更新日志互相一致。Mac 未纳入公开资产前，不把它写成已发布。
- [ ] Release 正文与 `nds-v1.0.0`、`nds-v1.1.0` 格式一致，明确 1.2 相对 1.1 的新增 DSi/TWL、缓存、跨存储卡游戏库、滚动列表、增强双屏、视频转换与游戏菜单；正式玩家介绍不提研究中的 FVDS，Android 用英文拼写，不增加多余的两 ROM 解释。
- [ ] 正式 1.2.0 Release 发出以后，再将此 Fork 根目录 README 替换为以 NDS 为主的全新 README，并最后更新原上游 PR #60 的描述、以已经存在的正式 Release 链接为准。

## M2 Pro 本机 DeepSeek GUI 验收结果（2026-10-27）

- [x] 原生 ARM64、Finder、双语、NVL1/NVA1、缺字、源码及存档保护、FFmpeg LGPL、ROM 配对本机 PASS；测试对象为 run `40505159429`。
- [x] **新版 macOS ARM64 GUI 复验 PASS（7/7）**：run `40513187447` / commit `1ce57b7ec72ea451dd53de1cc6d2139f877eb016`。英文下拉选项截断、日志重影、深色模式列表配色及 Finder `1.2.0` 版本显示均已修复；NVL1 输出与旧版逐字节一致。
- [x] 源分支已增补英文长选项的横向伸缩、固定日志配色/行距、按变化增量更新而非每 300ms 重绘，列表颜色一致性、Finder 版本 `1.2.0` 及辅助截图容错。
- [x] 用户本机 M2 Pro / macOS 27.0.1 **精准 UI 复核完成**：中英更多设置、日志滚动/刷新/切换、Finder 版本、深色对比度、复选框移除、NVL1 冒烟测试和截图失败安全降级全部通过。
- [ ] 浏览器下载的 Gatekeeper 隔离路径仍未实测；不能把 CLI 下载的 Finder 启动 PASS 等同于正式开发者签名或公证。
- [x] 两处**非阻断**文案的源码修补已完成：语言切换后从原始转换报告重生成“转换备注”；中文日志里的 `Staging:` 显示为“正在准备输出：”。对应 UI/i18n 回归通过：[40526678937](https://github.com/Blueteemo/CPyMO/actions/runs/40526678937)。**此前本机 7/7 PASS 属于旧 RC，新修改仅在源码，需待图标定稿后与最终套件一起构建并做一次轻量 GUI 冒烟测试，不要求再做完整验收。**

完整缺陷分析见 `MACOS-ARM64-RC-UI-REVIEW-20261027.md`。

## Apple Silicon / Rosetta 生命周期阻断检查（2026-10-27）

**Apple 官方明确：Rosetta 在 macOS 27 及更早版本普遍可用；从 macOS 28 起只针对部分停更旧游戏保留。** https://support.apple.com/zh-cn/102527

- [ ] **必须生成 ARM64 原生 macOS `.app`**，而不是让 Apple Silicon 玩家用 Intel RC + Rosetta。CI 采用 `macos-15` Apple Silicon M1 runner，PyInstaller 指定 `--target-arch arm64`，从源码编译 **ARM64** LGPL FFmpeg。整个应用／FFmpeg 的 Mach-O 架构必须明确包含 ARM64，且冻结进程实际运行视频转换与缺字回退。
- [x] **用户决定只提供 ARM64 原生 macOS 安装包。** 不再维护 Intel x86_64 公开发行附件；Intel Mac 可选择从源码运行转换工具。这一决定不涉及 NDS ROM 分裂或功能变更。
- [x] 正式发行仅列 `macOS-arm64`，不再提供 `macOS-x86_64`。不以 Intel + Rosetta 作为 Apple Silicon 支持；没有必要为 1.2 制作 Universal 2。
- [ ] **本地 DeepSeek 的最终验收对象是原生 ARM64 RC**（用户在 Apple Silicon Mac 上执行），需证明 Finder 的「显示简介」为 Apple 芯片应用、`file`／`lipo -archs` 校验 app 主程序及内嵌 FFmpeg；确认无需 Rosetta、正常打开 GUI、默认 NVL1 实际转码、NVA1、英文切换、缺字策略、文件与存档保护。不要求测试 Intel RC。
- [ ] 不自动在 macOS 28 已发布之前声称兼容性 100%；以 ARM64 独立本机运行消除 Rosetta 这一已知淘汰风险。Gatekeeper 签名／公证仍是另一个未解决的发行门槛。

## 发行依赖合规门槛（2026-10-27 增补）

- [x] 已识别并隔离原 macOS imageio-ffmpeg 构建携带 `--enable-nonfree` 的发行风险，旧 RC 不可公开发布。
- [x] 已记录原 Windows RC 的 Gyan GPL 预编译 FFmpeg 的真实构建信息，避免误将其当作 LGPL 可再分发版本。
- [x] Windows 与 macOS **RC** 均从 FFmpeg 7.1.5 对应源码构建，禁用 GPL 和 nonfree；含源包、构建参数、校验信息及许可。对应 https://github.com/Blueteemo/CPyMO/actions/runs/40494654625，公开发布前仍须对最终冻结文件进行一次许可与来源确认。
- [x] 两端 RC 均通过实际冻结工作进程视频转换、完整 ZIP 测试和两份 ROM 副本哈希一致性检查。详见 `RELEASE-ARTIFACT-MANIFEST-1.2.0.md`。
- [x] 已交付新 macOS LGPL RC 供本地 DeepSeek 测试，旧 nonfree 候选包明确作废。

## macOS DeepSeek 介入触发条件（优先 Apple Silicon 原生 RC）

在上述前三项全部满足后、**正式 GitHub Release 创建之前**，立即向用户交付用于本地 DeepSeek 的单次验收提示词。这是发版阻断检查点，不是无限期索要实机测试。Mac .app 的 2026-10-27 CI 自测成功仅代表构建与冻结工作进程经过合成媒体测试，**不能**代替最终 ROM 配套与 Mac Finder 用户体验验收。

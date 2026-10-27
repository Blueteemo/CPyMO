# CPyMO for NDS 1.2.0 — 发行工程与最后验收

## 最终桌面图标对应构建（2026-10-27）

- 成功运行：[paired CI #40536655608 / attempt 2](https://github.com/Blueteemo/CPyMO/actions/runs/40536655608/attempts/2)，源码提交 `71942aafb9a45029f7f595137fae80783f4c40d0`。首次 macOS 作业无任何日志步骤记录而在 15m01s 后取消；单独重试后真实 Apple Silicon 运行器正常启动且成功。
- 产物：macOS arm64 Actions artifact ID `12749434038`（外层 ZIP 55,040,816 字节）、Windows artifact ID `12748358625`（57,576,049 字节）、ROM ID `12748567395`、源码 ID `12747583214`。这些是 **Actions 外层 artifact** 的大小，不是最终分发 ZIP 的 SHA-256。
- macOS `.app` 的 `CFBundleIconFile`、生成 ICNS 一致性、ad-hoc 签名后资源哈希已由打包脚本验证。Windows PE 已经由 PyInstaller `--icon` 嵌入并成功完成 Windows 端的合成视频转换测试；未做 Windows 任务栏人工外观验证。
- 仍属于 **发行候选 RC**，不是公开 GitHub Release。公开发布前仍须完成视觉源图 MIT 署名、Gatekeeper 未公证说明、最终发布源代码与完整 SHA256SUMS、跨资产核对。

## 2026-10-28：最后界面微调与输入范围冻结

- 04 原图保持原始 1254×1254 RGBA 字节与 SHA-256 不变；Windows/macOS 图标仅将可见图案横向收窄 2.4% 并居中。NDS 32×32 图标保持原样。
- 依据原版 PyMO Android 安装说明，游戏数据应在解压后的游戏文件夹中，并无实际 APK 游戏包测试依据。发布版桌面转换器仅接收 ZIP/游戏文件夹，不再接受 Android APK 安装包；Android PyMO 游戏数据的转换支持不变。已更新 Android ZIP 测试和 APK 拒绝回归测试。
- 新图标的 Finder/Dock、Windows Shell 实际外观与最后文案需要轻量验收，不重复 DS/NTR、DSi/TWL 已通过的实机测试。

## 桌面转换器正式应用图标（2026-10-27）

- [x] 视觉方案：采用 10 月 25 日资料库的 **04_高清像素图标_透明外框.png**，而非 02 黑底版本。原件尺寸 1254×1254 RGBA，SHA-256 `3f110860ce4e242b79147ccd7b6c6d260efd1a232fa8039b41c02677aa3f8c62`。
- [x] 已在本地导出并校验 1024px PNG、64px GUI PNG、16–256px Windows ICO、支持 Retina 的 macOS ICNS；交付完整 04 原件与图标包。
- [x] Github 开发分支已加入确定性图标生成工具，Windows PyInstaller `--icon` / Tk、macOS ARM64 `--icon` / `CFBundleIconFile` 的接入及防默认图标校验；UI 回归测试已通过 [40533116706](https://github.com/Blueteemo/CPyMO/actions/runs/40533116706)。
- [x] 04 原始 PNG 已按原字节入库：`cpymo-tool/nds-converter/assets/converter-icon-source.png`（1,538,865 字节），SHA-256 `3f110860ce4e242b79147ccd7b6c6d260efd1a232fa8039b41c02677aa3f8c62`；CI 严格核验固定来源。
- [x] [paired CI #40536655608，attempt 2](https://github.com/Blueteemo/CPyMO/actions/runs/40536655608/attempts/2) **5/5 作业成功**：两端打包都强制 `--require-desktop-icon`。macOS 在 `.app` 内逐字节校验 `.icns`、写入 `CFBundleIconFile` 并在签名后复核；Windows 使用 PyInstaller `--icon`，但 Windows 外壳/任务栏的真人视觉检查尚未单独进行。
- [x] 已追溯高清图标的 PyMO 原始母题至 [pymo/pymo/images/pymo-icon.png](https://github.com/pymo/pymo/blob/master/images/pymo-icon.png)，查到该上游仓库 MIT LICENSE（Copyright (c) 2013 pymo），并在源码与两端打包流程内加入原许可证及“衍生/AI辅助图标”明确来源署名。未宣称独立原创原角色、亦未宣称上游背书。


此清单仅用于发版操作，优先级高于早期 1.2 研究与阶段性试验文档。
用户已完成 R4 Wood DS/NTR 和 DSpico DSi/TWL 的本地 ROM 实机验收；
不要再要求用户重复进行已经验收过的 NDS 游戏端测试。

## 必须完成的步骤

- [x] 生成且校验通用 `cpymo-nds.nds` 的 1.2 候选：NTR/TWL 使用同一 ROM，02-B 星夜背景 SHA-256 与实机验收素材一致，未保留 placeholder；后续冻结发布提交时须再次核对输出哈希。对应 CI：https://github.com/Blueteemo/CPyMO/actions/runs/40490746670
- [x] Windows Starter **RC** 嵌入相同 SHA-256 的通用 ROM；默认 NVL1、缺字自动方框、输入源完整性及实际 FFmpeg 视频转换的冻结工作进程测试通过。正式发行提交冻结后须重建并重复核验。
- [x] macOS Intel `.app` **RC** 完整打包并通过 CI 视频转换及配套 ROM 校验；附 FFmpeg 7.1.5 完整对应源码与 LGPL 编译参数。**尚未通过用户自己的 Finder/Apple Silicon 图形验收**。
- [x] 已在本轮发版会话向用户提供**经过核验的 macOS LGPL RC ZIP**（SHA-256 `d99ca80e64ecdb098cb4e116f4eea7625109dbb9869c597e70dd6f4176168546`）及可直接交给 DeepSeek 的独立中文验收提示词；明确要求验证 Finder、ZIP/目录（旧 RC 也测过 APK，正式版已移除该输入）、默认 NVL1/NVA1、缺字、源文件与存档保护、Apple Silicon/Rosetta，并禁止关闭 SIP 或 Gatekeeper。
- [x] 本机 DeepSeek 的 macOS ARM64 7/7 GUI 修复复验报告已经收到并审核（run `40513187447`）；现有最终图标 RC 已通过 macOS CI，图标的 Finder/Dock 外观仍需最后视觉观察，不必重复 NDS 实机验收。
- [ ] Windows/macOS 包、源码及许可证梳理、FFmpeg 构建信息、SHA256SUMS 与更新日志互相一致。Mac 未纳入公开资产前，不把它写成已发布。
- [x] Release 正文草稿已按 1.0/1.1 风格编写：准确描述 DSi/TWL、缓存、跨设备资料库、滚动与触控、三态双屏、NVL1/NVA1 视频、菜单存档，并明确输入支持 Android 游戏数据；排除研究中的 FVDS，补充桌面图标出处与 Mac 未公证说明。**附件下载 URL、最终 SHA256/文件大小仍待最后冻结后填入**。
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

- [x] 原生 macOS ARM64 `.app` 已构建：实际 Apple Silicon 本机 7/7 复验 PASS（上一候选），最新嵌入 04 图标的产物见 paired CI `40536655608`，无需 Rosetta，FFmpeg 与主程序均检查 ARM64 架构。
- [x] **用户决定只提供 ARM64 原生 macOS 安装包。** 不再维护 Intel x86_64 公开发行附件；Intel Mac 可选择从源码运行转换工具。这一决定不涉及 NDS ROM 分裂或功能变更。
- [x] 正式发行仅列 `macOS-arm64`，不再提供 `macOS-x86_64`。不以 Intel + Rosetta 作为 Apple Silicon 支持；没有必要为 1.2 制作 Universal 2。
- [x] 本地 DeepSeek 已独立完成原生 ARM64 Finder/GUI 核心和 7/7 UI 复验（在图标更换前）；最终图标版本由 CI 自动检查 `.icns`，仍需 Finder/Dock 图形展示复核，不能误称该视觉点已有人眼通过。
- [ ] 不自动在 macOS 28 已发布之前声称兼容性 100%；以 ARM64 独立本机运行消除 Rosetta 这一已知淘汰风险。Gatekeeper 签名／公证仍是另一个未解决的发行门槛。

## 发行依赖合规门槛（2026-10-27 增补）

- [x] 已识别并隔离原 macOS imageio-ffmpeg 构建携带 `--enable-nonfree` 的发行风险，旧 RC 不可公开发布。
- [x] 已记录原 Windows RC 的 Gyan GPL 预编译 FFmpeg 的真实构建信息，避免误将其当作 LGPL 可再分发版本。
- [x] Windows 与 macOS **RC** 均从 FFmpeg 7.1.5 对应源码构建，禁用 GPL 和 nonfree；含源包、构建参数、校验信息及许可。对应 https://github.com/Blueteemo/CPyMO/actions/runs/40494654625，公开发布前仍须对最终冻结文件进行一次许可与来源确认。
- [x] 两端 RC 均通过实际冻结工作进程视频转换、完整 ZIP 测试和两份 ROM 副本哈希一致性检查。详见 `RELEASE-ARTIFACT-MANIFEST-1.2.0.md`。
- [x] 已交付新 macOS LGPL RC 供本地 DeepSeek 测试，旧 nonfree 候选包明确作废。

## macOS DeepSeek 介入触发条件（优先 Apple Silicon 原生 RC）

在上述前三项全部满足后、**正式 GitHub Release 创建之前**，立即向用户交付用于本地 DeepSeek 的单次验收提示词。这是发版阻断检查点，不是无限期索要实机测试。Mac .app 的 2026-10-27 CI 自测成功仅代表构建与冻结工作进程经过合成媒体测试，**不能**代替最终 ROM 配套与 Mac Finder 用户体验验收。

# CPyMO for NDS 1.2.0 — 发行工程与最后验收

此清单仅用于发版操作，优先级高于早期 1.2 研究与阶段性试验文档。
用户已完成 R4 Wood DS/NTR 和 DSpico DSi/TWL 的本地 ROM 实机验收；
不要再要求用户重复进行已经验收过的 NDS 游戏端测试。

## 必须完成的步骤

- [x] 生成且校验通用 `cpymo-nds.nds` 的 1.2 候选：NTR/TWL 使用同一 ROM，02-B 星夜背景 SHA-256 与实机验收素材一致，未保留 placeholder；后续冻结发布提交时须再次核对输出哈希。对应 CI：https://github.com/Blueteemo/CPyMO/actions/runs/40490746670
- [ ] Windows Starter 嵌入相同 SHA-256 的正式 ROM；GUI 默认 NVL1 视频转换；缺字自动以 `□` 显示并生成警告，原始脚本与存档不得改写；完整验证 FFmpeg 视频解码能力。
- [ ] 生成 macOS 图形转换助手 .app 发行候选。使用与最终 ROM 相同的版本／素材，复核 ROM SHA；验证可下载包的展开、冻结进程处理合成视频、FFmpeg 随包、缺字警告、输出完整性及第三方许可。
- [ ] **强制停在此处通知用户：请本地 DeepSeek 在用户自己的 Mac 上验收 macOS 转换器。** 只有最终候选 macOS 安装包和配套引擎真正就绪时才提出；不要拿尚未封装正式星夜素材的实验 ROM 包要求实测。通知时提供实际可访问的候选包、SHA256 以及可直接复制给本地 DeepSeek 的完整中文提示词。提示词要求 DeepSeek 尝试 Finder 启动 .app、输入 ZIP/APK/游戏目录、视频默认 NVL1/NVA1 与可主动跳过、缺字自动替换方框并警告、保护脚本和旧存档、输出 TF 根目录规范、查看日志并报告复现步骤，不直接改项目源码。特别注意 Mac/Apple Silicon、Gatekeeper 及任何解压/启动安全警告；不得建议关闭 SIP、Gatekeeper。
- [ ] 收到 DeepSeek 的 Mac 实测回报并复核。未通过或有 blocker 时修正候选后再决策；不将内部 macOS CI 冒充用户本机 Finder 验收。若最终无法满足 macOS 发行门槛，Mac 版本可延后，但须明示。
- [ ] Windows/macOS 包、源码及许可证梳理、FFmpeg 构建信息、SHA256SUMS 与更新日志互相一致。Mac 未纳入公开资产前，不把它写成已发布。
- [ ] Release 正文与 `nds-v1.0.0`、`nds-v1.1.0` 格式一致，明确 1.2 相对 1.1 的新增 DSi/TWL、缓存、跨存储卡游戏库、滚动列表、增强双屏、视频转换与游戏菜单；正式玩家介绍不提研究中的 FVDS，Android 用英文拼写，不增加多余的两 ROM 解释。
- [ ] 正式 1.2.0 Release 发出以后，再将此 Fork 根目录 README 替换为以 NDS 为主的全新 README，并最后更新原上游 PR #60 的描述、以已经存在的正式 Release 链接为准。

## 发行依赖合规门槛（2026-10-27 增补）

- [x] 已识别并隔离原 macOS imageio-ffmpeg 构建携带 `--enable-nonfree` 的发行风险，旧 RC 不可公开发布。
- [x] 已记录原 Windows RC 的 Gyan GPL 预编译 FFmpeg 的真实构建信息，避免误将其当作 LGPL 可再分发版本。
- [ ] **Windows 与 macOS 的正式 RC** 改由 FFmpeg 7.1.5 完整对应源码构建（LGPL，不启用 GPL/nonfree），同时打包完整源文件、构建参数、校验信息和许可。
- [ ] 新来源合法的两个系统安装包均通过真实冻结工作进程视频转换及 ROM 哈希一致性检查。
- [ ] 只把全部通过上述门槛的新 macOS 包交给本地 DeepSeek 验收；不再使用旧的 nonfree 工具包。

## macOS DeepSeek 介入触发条件

在上述前三项全部满足后、**正式 GitHub Release 创建之前**，立即向用户交付用于本地 DeepSeek 的单次验收提示词。这是发版阻断检查点，不是无限期索要实机测试。Mac .app 的 2026-10-27 CI 自测成功仅代表构建与冻结工作进程经过合成媒体测试，**不能**代替最终 ROM 配套与 Mac Finder 用户体验验收。

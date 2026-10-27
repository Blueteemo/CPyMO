# CPyMO for NDS 1.2.0 — macOS ARM64 本机验收交接

> **仅供发行前测试。** 以前的 macOS Intel / Rosetta 测试包作废，新的正式发行方案只提供原生 Apple Silicon (ARM64) 安装包。不可把 Intel CI 或 Intel 应用通过 Rosetta 能运行当成 ARM64 通过。不要公开发布或修改仓库。

## 输入资料

- GitHub: https://github.com/Blueteemo/CPyMO
- 发版候选 Actions： https://github.com/Blueteemo/CPyMO/actions/workflows/nds-v12-release-candidate.yml
- 测试对象：`CPyMO-for-NDS-1.2.0-Starter-macOS-arm64-RC.zip`（**必须是最近一次 ARM64 原生构建通过之后下载的对应包**）
- ROM：一个 `cpymo-nds.nds`，同时供 DS/NTR 与 DSi/TWL 使用。已验收图片应随 ROM 内嵌，SHA-256 为 `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31`（候选阶段；后续冻结源提交若改变必须同步核对）。
- 新包已内置从源码构建的 LGPL FFmpeg 及其同版本源码，不应该依赖系统 Homebrew、Rosetta 或另装 Python。此候选未 Apple Developer ID 签名与公证。

## 可以直接交给本地 DeepSeek 的任务

请在我的 Apple Silicon Mac 上对我提供的 **CPyMO for NDS 1.2.0 macOS ARM64 RC** 进行真实本机验收，并输出精简而可复核的中文报告。

**界限：** 只测试解压到独立工作目录的候选包；不修改源码、不安装多余环境、不覆盖任何现有 PyMO 游戏数据或存档、不写入商业游戏。不要上传我的游戏文件到网络。禁止禁用 SIP、Gatekeeper 或清除系统全局隔离策略；若 Finder 显示开发者未经验证，记录原文和系统版本，仅使用 macOS 官方提供的本机单个应用审查及放行方式（需征得我同意），无法安全打开则报告阻碍。

请逐项核验：

1. **平台与架构。** 报告 macOS 版本、Apple 芯片型号和 `uname -m`；使用 `file`、`lipo -archs` 验证 `CPyMO-NDS-Converter.app/Contents/MacOS/CPyMO-NDS-Converter` 与应用内 `Contents/Frameworks/ffmpeg`（可能位于 Resources）的 ARM64 切片。若任何必要二进制仅为 x86_64，直接标记阻断。无需 Rosetta，也不要尝试安装它。
2. **安装包完整性。** 列出 ZIP 内顶级文件；读取 `VERSION.json`，确认 `macos_architecture=arm64`，对比外置与应用内置的 `cpymo-nds.nds` 的 SHA-256 是否一致。查看 `FFmpeg-Corresponding-Source` 存在源码压缩包、构建参数和许可证；内置 FFmpeg 的 `-buildconf` 不得包含 `--enable-nonfree`／`--enable-gpl`。
3. **Finder GUI。** 在 Finder 中直接打开 .app（不是只运行命令行工作进程），检查首屏所有字段有无溢出／遮挡／空白。切换「简体中文 / English」，确认全部核心选项、按钮、状态说明切换正确，英文不溢出；关闭再打开后记住语言。检查取消、错误弹窗及打开结果文件夹等功能。
4. **输入选择。** 使用你在隔离临时目录准备的合法最小 PyMO 测试样本，分别尝试游戏目录、ZIP，能构造 APK 则一并检查。不碰我既有任何游戏包和个人存档。选择源目录时不能将输出目录设在其中。
5. **视频。** 使用原创测试画面+音轨做 1 段极短 MP4：默认不操作视频选项时实际输出 NVL1；手工选 NVA1 后输出 NVA1；主动选「跳过视频」时不生成播放缓存。检查转换报告与实际视频帧数量，失败时保留日志，勿因某种编码不可用就静默声称成功。
6. **缺字与存档保护。** 在原始测试脚本中加入一个未收录 Unicode 字符，确认默认只警告并显示方框，不停止转换、不覆写原脚本；检查旧 `save/` 未被触及；输出的 NDS 游戏目录结构和 ROM 应位于合适位置。
7. **第三方依赖。** 从应用本身完成上述工作，不调用系统的非捆绑 FFmpeg；验证程序未依赖 Intel / Rosetta。遇到官方系统提示，截图或记录文字，不作未经许可的安全设置修改。

最终请提供：`PASS / FAIL / BLOCKED` 总结、每个测试点的结论、SHA-256、体系架构信息、可复现的具体问题（包括日志路径和截图）；如果所有测试通过，明确写「ARM64 原生 macOS 发行候选已通过本机验收」。

本地验收仅认证此一候选包和测试条件，不代表所有 PyMO 游戏、所有编码和所有 macOS 版本百分之百兼容。**不需要复测 NDS 掌机，掌机实机已另行通过。**

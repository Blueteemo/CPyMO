# CPyMO for NDS 1.2.0 · macOS ARM64 RC 本机 UI 验收复核

## 本机测试报告结论（2026-10-27）

测试对象：GitHub Actions run `40505159429` 的原生 ARM64 RC，Intel/Rosetta 旧包不参与验收。**本机总结论 FAIL，只涉及 UI 展示，不能作为已通过正式版**。

- **PASS：** M2 Pro、macOS 27.0.1 的原生 ARM64（无 Rosetta 转译），ZIP 与三处 ROM 配对，FFmpeg 7.1.5 LGPL，Finder 打开、简体中文/English 及语言记忆；目录/ZIP/APK 合成输入；默认 NVL1、NVA1、跳过视频；缺字方框并保持脚本哈希、存档不覆盖；转换错误处理和取消恢复。
- **FAIL：** English `Lossless voice and image cache compression (recommended)` 在 `width=47` 的下拉框内截断；转换结束后日志多行文字重影/重叠。均为可见的用户体验问题，必须修正并复核。
- **低优先级：** 原游戏列表在深色外观下与浅色页面形成黑底反差；Finder `Info.plist` 为 0.0.0 而非 1.2.0；测试辅助的 macOS 截图 API 可能失败；CLI 下载未带 quarantine，因此**没有覆盖浏览器下载导致的 Gatekeeper 提示**。

## 已提交的修复（后续源分支版本，并非该 RC 的回溯结果）

- 主界面隐藏并自动应用“精简重复媒体”逻辑，非专业玩家无需看到；缓存压缩移动到可展开的“更多设置”，默认保持推荐选项。
- 英文长的缓存压缩 Combobox 改成 `fill='x', expand=True` 与较宽值，打开更多设置时保留完整文案。
- macOS Text 日志区显式设置高对比的深色前景、浅色背景、选择高亮以及更宽行距；使用 Menlo（及系统 CJK 字符回退）。将每 300ms 清空重写改为**仅内容变化时增量追加**，避免 Tk 在 macOS 重复布局和刷新引发绘制重影。
- Listbox 明确设置背景/前景/选中配色，防止深色模式在浅色页面中出现系统黑底。
- 完成 PyInstaller 后将 `CFBundleShortVersionString`、`CFBundleVersion` 改为 `1.2.0`，重新 ad-hoc 签名并验证；**不是** Apple Developer ID 正式签名或公证。
- 辅助 GUI 截图权限或 API 失败不再丢失转换结果，也不会让验收长期停在截图环节。

## 发版约束与复核

- 修复完成仍需**同一新候选包**的自动化测试和用户本机 UI 精简复核，不能拿此前旧 RC 的核心 PASS 直接覆盖新版 RC 的 UI 结果。
- 推荐本机复核范围：M2 Pro Finder 打开并查看 `Info.plist=1.2.0`、English「更多设置」的压缩选项完整展示、中英完成转换后的 3–5 行日志不重叠、暗色系统外观下日志与游戏列表对比度、语言切换和状态栏。可用自建两秒短片重新跑一次 NVL1 冒烟测试；**不需**重做此前全部游戏/ROM/FFmpeg 测试。
- 仍未实际复核浏览器下载带 quarantine 的 Gatekeeper 用户路径。没有 Apple 开发者签名及公证时，不得指示关闭系统全局安全保护。
- 已定稿的 NDS/DSi ROM 不因电脑 GUI 样式修复重新要求掌机实机测试。

报告来源：用户上传的 `CPyMO_macOS_ARM64_1.2_RC_本地验收报告.md`；原本机日志及截图在用户的本机测试目录，仓库内无用户个人测试数据。

## 第二轮本机 UI 复验：7/7 PASS（2026-10-27）

用户提供独立 Markdown 复验报告 `CPyMO_macOS_ARM64_1.2_RC_UI复验报告.md`，测试对象为 [GitHub Actions run 40513187447](https://github.com/Blueteemo/CPyMO/actions/runs/40513187447)，源提交 `1ce57b7ec72ea451dd53de1cc6d2139f877eb016`，对应 `macos-rc` 成功。应以本节**最新结果**覆盖上文旧 RC 的 FAIL 状态，但保留旧记录作为问题发现和修复过程。

- **总评：PASS。** 7/7 UI 复验点全部通过：英文高级选项完整显示；中英日志滚动、刷新、语言切换无重影；深色模式列表和日志高对比；Finder 版本号显示 1.2.0；隐藏「精简重复媒体」复选框；NVL1 冒烟转换；系统截图失败安全降级。
- **NVL1 无回归：** 自建两秒样本、24 帧、442,873 字节，视频 SHA-256 `86424a401cb45381bc58284e12566fec5e3ef1c03474491c661df1efc1f91ba4`，与上一版逐字节一致。输入数据和个人存档未被更改。
- **正式候选 ROM：** SHA-256 `7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31`；外置与应用内一致。
- **候选内层 ZIP：** `CPyMO-for-NDS-1.2.0-Starter-macOS-arm64-RC.zip`，53,181,202 字节，SHA-256 `202809b0d88bcf47f7e8b38d953e2918ed2aa99722bc9334e674f3981b399a04`。
- **非阻断遗留：** 已产生的转换备注在界面语言切换后不重新翻译；中文日志仍出现 `Staging:` 英文。
- **单独存在的发版限制：** Mac `.app` 仍为 ad-hoc 签名，无 Apple Developer ID 签名或公证；本机测试经 CLI 下载不带 `quarantine`，**没有测试浏览器下载触发的 Gatekeeper 用户体验**。此项不能由 GUI 7/7 PASS 自动覆盖。

7/7 仅指第二轮约定的 GUI 复验范围，不表示所有 Mac、所有商业游戏、所有 macOS 版本或软件分发安全均已认证。无需再重做 NDS/DSi 掌机验收。

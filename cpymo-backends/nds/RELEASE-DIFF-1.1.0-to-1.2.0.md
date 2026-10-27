# CPyMO for NDS 1.1.0 → 1.2.0 源码差异核对

对比基准：正式标签 `nds-v1.1.0` 与已验收分支 `fix/nds-v1.2-pico-acceptance-20261027`（在本档第一次审计时共约 383 个新增提交，GitHub Compare 返回 **113 个变更文件**：91 新增、22 修改；文档／测试／研究代码也计入，不等于 113 个面向玩家的新功能）。此文是 **内部审核依据**，不是玩家发行说明。

## 需要写入 1.2.0 发行更新的实际功能

| 领域 | 相较 1.1.0 的新增内容 | 对应源码／证据 | 发布措辞边界 |
|---|---|---|---|
| **原生 DSi/TWL 运行** | 通过 `isDSiMode()` 识别实际 TWL 模式；内存窗口及模式标签按实际运行模式显示，兼容普通 NTR。用户分别在 R4 Wood DS 和 DSpico DSi 模式验收 | `source/main.c`（`nds_recent_image_budget`、`nds_runtime_state`）、`source/launcher.h`、`build.py` | 应写「新增 DSi 模式（TWL）支持」。不能声称发行的是 **DSiWare 应用**，也不能仅凭 ROM Header 推断已在 TWL 运行 |
| **DSi 条件式资源缓存** | 旧版 DS：6 个近期立绘缓存槽、384 KiB 上限；真正的 TWL 且堆窗口符合条件时：最多 12 个槽、2 MiB 上限，可共同复用立绘和背景资源 | `source/main.c` 中 `RECENT_LIMIT_TWL`、`recent_capacity`、`nds_bg_cache_enabled`、`recent_find/store` | 这是容量上限／有条件的命中优化，不是每次预分配 2 MiB，也不保证所有游戏等比例加速 |
| **DSi 双存储设备游戏库** | TWL 模式从当前设备延伸扫描可访问的另一 `fat:`／`sd:` 设备，绝对路径跟随游戏，存档与缓存仍在原设备 | `source/launcher.h` 的 `launcher_scan_other_card`、`launcher_valid_game_path`、`launcher_scan` | 访问可能受启动器和存储方向限制；不是无条件双卡同时可读 |
| **大游戏库滚动与触控** | 超过 4 个游戏时出现细滚动条，列表随方向键连续滚动、支持手指／触控笔拖动滚动条，保留原卡片样式及双击启动 | `source/launcher.h` 的 `launcher_scroll_*()`、`launcher_choose` | 不声称适用于无限数量游戏；代码仍限制最多 64 条游戏条目 |
| **三态显示／增强双屏** | L 循环 经典上屏→经典下屏→增强双屏；增强模式场景在上、对白／选项／菜单在下，适配物理触控；按游戏保存模式。原游戏标题阶段保持原布局 | `source/dual-screen-nds.h`、`dual-input-nds.h`、`dual-layout-nds.h`、`dual-title-nds.h`、`display-mode-prefs-nds.h`，`source/main.c` | 不说 3DS 本体移植、不说所有游戏排版绝对相同 |
| **增强双屏内嵌美术** | 256×192 RGB555 星夜背景随最终单 ROM 分发；普通对白及确认／选项共用背景，交互时适度变暗，视频时只显示背景不保留旧对白 | `source/nds-picture-bg.h`、`main.c`、`movie-lz4-r7.h`、`tools/patch-rom-bg.py` | 原始 CI 基础 ROM 包含待填充字节，**正式流水线须嵌入美术并校验**，不可发布占位版本 |
| **玩家可选视频功能** | 1.1 的发行转换器省略视频；1.2 GUI／CLI 支持 FFmpeg 转码普通片源至可播放的 NVL1（PCM）或 NVA1（NAR2）NDS 缓存。支持自然结束／B 跳过／返回剧情，兼顾无声视频 | `cpymo-tool/nds-converter/app.py`、`convert.py`、`helper_core.py`、`video_nvl.py`；`source/movie-lz4-r7.h` | 1.1 已有部分实验性运行时 NVL 路径，不应说 1.2 才首次出现所有解码代码；重点是**从原片输入到玩家可用转换与播放的完整工作流**；视频转换仍为可选，默认可跳过 |
| **菜单、存档与 UX** | X 菜单新增「退出到游戏库」确认；设置值居中，存档目录不存在时自动创建，增强模式存读档反馈可见，游戏库底部字体位置微调 | `source/cpymo_rmenu.c`、`cpymo_config_ui.c`、`save-dir-nds.h`、`launcher.h`、`main.c` | 原有快速存读档并非 1.2 新增；应写修正／优化 |
| **显示与国际化修正** | 保留 1.1 四语选择；增补 1.2 文案所需的 ROM 内建字符，游戏列表加载过场采用保存的语言偏好，原生启动 Banner 三行包括 1.2.0 和移植署名 | `source/ui-fallback-font.h`、`ui-language-nds.c`、`ui-language-extra.h`、`rom_banner.py`、`build.py` | 四语本身是 1.1 已有功能，不应说本版才首次支持四语 |

## 1.1 已经有的能力，1.2 不能重复当作新增

- 多游戏启动器、图标、标题简介、首次转换、S60v3／S60v5／安卓 PyMO 输入。
- 简中／繁中／英语／日语的 UI、原生点阵字库、时间显示、错误页面。
- START／SELECT 标准存读档、Y 副屏切换、原有 2 种屏幕布局、合盖处理和基本输入。
- 游戏缓存格式 6、旧缓存 1–5 的兼容、立绘及语音的既有压缩与去重工作。
- 原有 NVL1/NVA1 解码**研究基础**（1.1 缺玩家可用的视频转码发行链）。
- 游戏原文、剧本、存档格式及上游通用 `cpymo/` 源码没有被此次比较中的改动文件修改。

## 不得混入 1.2 玩家发行介绍

- FVDS 全片编码／解码的研究文件、预检、非公开参考代码与未完整接入的模块。
- 尚未验证的「DSiWare 安装包／官方 DSi 商店应用」能力。
- 未做公开实机确认的性能倍数、所有 ROM／所有游戏／所有烧录卡的完全兼容承诺。
- CI 里尚未注入星夜美术的开发用 ROM；没有打包许可证材料的 Windows 实验构建。

## 下一步发行门槛

1. 以一个已验收、可在 NTR/TWL 运行的通用 ROM 产出正式 `cpymo-nds.nds`；内嵌已验收的星夜美术并逐字节复核。
2. Windows Starter 必须嵌入**同一文件 SHA-256** 的正式 ROM，并验证冻结 GUI／转换助手和视频依赖。
3. 只按准确支持范围更新主 README、后端和转换器文档；玩家 Release 继续沿用 1.0／1.1 的六段式，重点说明相对 1.1 的差异。
4. 生成最终对应源码包、第三方许可证资料和 SHA256SUMS；避免把私人游戏、视频或不明许可实验代码打包。
5. 最终发行包校验成功后再创建 `nds-v1.2.0` 公开 Release。

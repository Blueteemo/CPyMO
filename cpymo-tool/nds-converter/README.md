# CPyMO NDS 转换助手 1.1.0

NDS 移植：千岚之夏。独立转换助手 MIT；引擎 AGPL-3.0-or-later；字体与依赖保留各自许可。

## 给玩家

完整解压 Windows Starter，双击“启动转换器.bat”，或运行“转换器/Converter.exe”。
无需安装 Python、FFmpeg 或编译器。选择游戏 ZIP/APK 或含 gameconfig.txt 的文件夹，
支持嵌套 ZIP 与 S60v3 / S60v5 / 安卓 PyMO 数据（指输入数据平台，非电脑系统）。
同包多版本优先推荐 S60v3。选择新的输出位置，开始转换，结束后阅读兼容性警告与安装必读。

把输出的 cpymo-nds.nds 和 pymo 文件夹复制到 TF 根目录。路径应为
`pymo/games/<游戏目录>/gameconfig.txt`，旁边有 script、nds-cache 和 save。
保留 R4 内核。已有同名游戏先保留自己的 save，不要用新输出的空 save 覆盖。
转换助手在电脑使用，不需要复制进 TF。本包不附游戏。

默认精简已有缓存的重复原媒体，无损压缩语音与适合的图片；BGM/音效保持 PCM，兼顾读取和存储。
无损针对 NDS 缓存：图像先缩放为 DS 尺寸 RGB555，音频先转为 22050 Hz 单声道 PCM16。
缓存压缩不会在此基础上继续降低像素或采样质量；不承诺比所有原版平台包更小。
可选择不压缩缓存作对照；全部音频压缩是实验项，不属于默认性能基线。
启用缓存压缩会同时精简重复媒体。输出专用于 NDS，保留原包供其他平台使用。

视频资源省略，脚本中的视频指令保留并由引擎安全略过。原始包、脚本和定稿汉化不修改，源存档不导入。
转换通过不等于全剧情或全部烧录卡认证。缺资源、未知指令和缺字策略记录在
`nds-cache/manifest.json` 及转换结果中。GUI 默认缺字显示 □ 并记录，脚本文字不改。
加密/打包脚本和其他游戏引擎不支持。

## 1.2 集成分支：可播放视频转换（开发验收功能）

**1.1.0 正式版不受影响。** 原有默认行为仍然是省略视频、原样保留脚本；
1.2 集成分支新提供可选的 **NVL1**（22050 Hz PCM16）与 **NVA1**（NAR2
无损压缩音频）视频转换，在 GUI 的“游戏视频”下拉框中选择。默认
12 FPS、256×192 RGB555；保留原始画面比例，居中补黑边，不随意拉伸。
字幕、脚本与存档策略不改；无音轨视频补零音频。

CLI 用法（在正式发布前需搭配同分支 ROM）：



```sh
python cpymo-tool/nds-converter/convert.py convert 原游戏目录 新输出目录 --video-mode nvl1 --cache-only --missing-glyph-policy box
# 压缩视频音轨（更省空间，但建议先做实机性能验证）
python cpymo-tool/nds-converter/convert.py convert 原游戏目录 另一新目录 --video-mode nva1 --video-fps 12
```

识别游戏目录内的 \`video/\`、\`movie/\`、\`videos/\`、\`movies/\`
普通视频（FFmpeg 支持的 MP4/AVI/MKV/MOV 等）。输出为
\`nds-cache/video/<哈希前缀>/<视频名哈希>.nvl\`；原 PyMO \`#movie\` 指令
无需修改。全程校验帧的 Adler32 与像素还原，音轨严格按视频帧数对齐；
不支持的视频会在审计中继续标注为未转换。单段视频受现有 NDS 播放器
10 分钟限制。生成缓存及主机验证通过**不等于** NTR/DSi 实机视频性能、
跳过恢复、所有原始游戏视频的兼容性已验收。

新的 \`video_nvl.py\` 和本路径下的 NVL 视频转换测试不使用
Gericom/FastVideoDSPlayer 的专有或未授权查表。FVDS 独立解码仍存在
发布许可门槛，不能混入公共 1.2 源码或 ROM。


## 从源码运行与构建

Python 3.10+，安装 requirements.txt，另备 FFmpeg；完整仓库内运行 `python app.py`。
Windows 提供免安装包，macOS/Linux 可运行源码 CLI，不提供原生免安装包。

```sh
python -m pip install -r cpymo-tool/nds-converter/requirements.txt
python cpymo-tool/nds-converter/convert.py convert /path/to/game /path/to/new-output --ffmpeg /path/to/ffmpeg --missing-glyph-policy box --cache-only --audio-lossless voice --compress-images
python cpymo-tool/nds-converter/convert.py verify /path/to/new-output
```

CLI 默认缺字报错且保留便于对照的原媒体/未压缩缓存；上述参数与 GUI 推荐路径一致。
缓存格式6按哈希前两位分目录，兼容引擎旧格式1～5；`--cache-layout flat` 用于对照测试。
图像/音频去重不改脚本资源名称。保留 fonts/ 的字体文件和全部许可，不要只拷单个 Python 文件。
版本从 cpymo-backends/nds/release.json 读取。

构建 Windows 包：先构建 ROM，安装 requirements-build.txt，使用可信 Windows x86_64 C 编译器构建加速库：

```sh
python build_native.py --cc /path/to/x86_64-w64-mingw32-gcc.exe
python build_portable.py --ffmpeg /path/to/ffmpeg.exe --output /path/to/new-portable
```

加速库可选，源码模式缺失时使用 Python 实现；公开 Windows 包包含该库。
打包脚本嵌入并检查同一 ROM 与版本元数据。发布时还需附玩家必读、第三方许可和完整对应源码。
FFmpeg 独立进程运行，不链接进 MIT 转换器；tools/ 提供音频专用构建脚本。
发行页 Sources.zip 包含相应 FFmpeg 官方源码、构建配置及工具链许可。

回归：`python -m unittest discover -s cpymo-tool/nds-converter/tests -v`，可用 NDS_TEST_FFMPEG 指定程序。
app.py 的 `--self-test JSON` 通过真实 GUI 与工作进程完成转换，未指定压缩字段时使用实际界面默认值。

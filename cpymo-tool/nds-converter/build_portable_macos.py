"""Build a self-contained macOS CPyMO NDS conversion assistant (.app).

Development candidate until native macOS bundle, third-party FFmpeg licensing
and Gatekeeper distribution are validated. Must be built ON macOS with an
explicit final ROM; never substitutes an unrelated local 1.1 engine.
"""
from __future__ import annotations
from pathlib import Path
import argparse
import hashlib
import importlib
import json
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
BACKEND = HERE.parents[1] / "cpymo-backends/nds"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ffmpeg", type=Path, required=True)
    parser.add_argument("--rom", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("Build native macOS applications on a macOS host.")
    for module in ("PyInstaller", "PIL.Image", "lz4.block", "tkinter"):
        try:
            importlib.import_module(module)
        except ImportError as exc:
            parser.error(f"Missing package {module}: {exc}")
    output = args.output.resolve()
    rom = args.rom.resolve()
    ffmpeg = args.ffmpeg.resolve()
    if output.exists() or not rom.is_file() or not ffmpeg.is_file():
        parser.error("Choose a NEW output directory and valid ROM / FFmpeg inputs.")
    from video_nvl import validate_ffmpeg
    validate_ffmpeg(str(ffmpeg))
    metadata = json.loads((BACKEND / "release.json").read_text(encoding="utf8"))
    if metadata["version"] != (BACKEND / "VERSION").read_text().strip():
        parser.error("Engine version metadata mismatch.")
    output.mkdir(parents=True)
    build = output / "_build"
    command = [sys.executable, "-m", "PyInstaller",
               "--noconfirm", "--clean", "--onedir", "--windowed",
               "--name", "CPyMO-NDS-Converter",
               "--osx-bundle-identifier", "dev.blueteemo.cpymo-nds-converter",
               "--distpath", str(build / "dist"),
               "--workpath", str(build / "temp"),
               "--specpath", str(build),
               "--add-data", f"{HERE / 'fonts'}:fonts",
               "--add-data", f"{BACKEND / 'release.json'}:.",
               "--add-data", f"{rom}:.",
               "--add-binary", f"{ffmpeg}:.",
               "--exclude-module", "numpy", "--exclude-module", "scipy",
               "--exclude-module", "matplotlib", "--exclude-module", "pytest",
               str(HERE / "app.py")]
    # The runtime uses sys._MEIPASS for the bundled FFmpeg and ROM. macOS's
    # app contains Resources/Frameworks cross-links; inspect both locations.
    with (output / "build.log").open("w", encoding="utf8") as log:
        subprocess.run(command, check=True, stdout=log, stderr=subprocess.STDOUT)
    app = build / "dist/CPyMO-NDS-Converter.app"
    if not app.is_dir():
        raise RuntimeError("Missing .app from PyInstaller")
    for resource in ("cpymo-nds.nds", ffmpeg.name, "release.json"):
        present = any((app / location / resource).is_file() for location in
                      ("Contents/Resources", "Contents/Frameworks", "Contents/MacOS"))
        if not present:
            raise RuntimeError("Missing bundled resource " + resource)
    internal_rom = next(p for p in (app / "Contents/Resources/cpymo-nds.nds",
                                  app / "Contents/Frameworks/cpymo-nds.nds") if p.is_file())
    if digest(internal_rom) != digest(rom):
        raise RuntimeError("Bundled ROM does not match the supplied input")
    shutil.copytree(app, output / app.name, symlinks=True)
    shutil.copy2(rom, output / "cpymo-nds.nds")
    shutil.rmtree(build)
    metadata.update(rom_sha256=digest(rom), ffmpeg_sha256=digest(ffmpeg),
                    macos_bundle_validation="resources matched; not signed/notarized")
    (output / "VERSION.json").write_text(json.dumps(metadata, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    (output / "README-macOS.txt").write_text(
        "CPyMO NDS 转换助手 macOS 开发候选包（非正式发行版）\n"
        "打开 CPyMO-NDS-Converter.app 转换原版 PyMO 游戏。\n"
        "如遇到 macOS 安全提示，不要禁用 Gatekeeper 或系统完整性保护。\n"
        "此开发候选版没有 Apple Developer ID 签名及公证，正式公开发布前\n"
        "还需要原生 GUI 测试、相应许可证材料及平台发行安全审查。\n"
        "可运行游戏程序为同目录 cpymo-nds.nds，不含商业游戏。\n",
        encoding="utf8")
    print(json.dumps({"version": metadata["version"], "rom_sha256": digest(rom),
                      "app": str(output / app.name), "macos_candidate": True},
                     ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()

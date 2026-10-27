#!/usr/bin/env python3
"""Prepare Windows/macOS CPyMO converter icons from ONE approved RGBA PNG.

The full-resolution approved image is kept in the project archive. For
reproducible release packaging, copy that exact source PNG to
cpymo-tool/nds-converter/assets/converter-icon.png before building.
This script does not modify the Nintendo DS game's original 32x32 icon.
"""
from __future__ import annotations

import argparse
import json
import platform
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory

from PIL import Image


WINDOWS_SIZES = ((16, 16), (24, 24), (32, 32), (48, 48),
                 (64, 64), (128, 128), (256, 256))
MAC_SIZES = (
    ("icon_16x16.png", 16),
    ("icon_16x16@2x.png", 32),
    ("icon_32x32.png", 32),
    ("icon_32x32@2x.png", 64),
    ("icon_128x128.png", 128),
    ("icon_128x128@2x.png", 256),
    ("icon_256x256.png", 256),
    ("icon_256x256@2x.png", 512),
    ("icon_512x512.png", 512),
    ("icon_512x512@2x.png", 1024),
)


def generate(source: Path, output: Path, target: str) -> dict:
    source = source.resolve()
    if not source.is_file():
        raise FileNotFoundError(f"Approved converter icon missing: {source}")
    with Image.open(source) as opened:
        image = opened.convert("RGBA")
    if image.width != image.height or image.width < 1024:
        raise ValueError("Converter master icon must be square and at least 1024px")
    alpha = image.getchannel("A")
    lo, hi = alpha.getextrema()
    if lo != 0 or hi != 255:
        raise ValueError("Approved icon must retain its transparent outer border")
    output.mkdir(parents=True, exist_ok=True)
    if target == "windows":
        icon = output / "converter-icon.ico"
        image.save(icon, format="ICO", sizes=WINDOWS_SIZES,
                   append_images=[], bitmap_format="png")
    elif target == "macos":
        if platform.system() != "Darwin":
            raise RuntimeError("macOS iconutil must run on a native macOS host")
        icon = output / "converter-icon.icns"
        with TemporaryDirectory(prefix="cpymo-icon-") as tmp:
            iconset = Path(tmp) / "converter.iconset"
            iconset.mkdir()
            for filename, size in MAC_SIZES:
                image.resize((size, size), Image.Resampling.LANCZOS).save(
                    iconset / filename, format="PNG", optimize=True)
            subprocess.run(["iconutil", "-c", "icns", "-o",
                            str(icon), str(iconset)], check=True)
    else:
        raise ValueError(f"Unknown target: {target}")
    if not icon.is_file() or icon.stat().st_size < 1000:
        raise RuntimeError("Desktop icon conversion produced an invalid file")
    return {"master": str(source), "target": target,
            "icon": str(icon), "bytes": icon.stat().st_size}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--target", choices=["windows", "macos"], required=True)
    args = parser.parse_args()
    print(json.dumps(generate(args.source, args.output, args.target), indent=2))


if __name__ == "__main__":
    main()

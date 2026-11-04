#!/usr/bin/env python3
"""Prepare desktop converter icons from the approved PyMO 04 artwork.

Only image scaling and standard icon-container conversion; never modifies the
NDS ROM banner icon. The canonical original is the Library archive's
04_高清像素图标_透明外框.png (1254x1254, RGBA).
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image
from array import array
from math import sqrt

SOURCE_SHA256 = "3f110860ce4e242b79147ccd7b6c6d260efd1a232fa8039b41c02677aa3f8c62"
# 2026-11-01: the user's final approved 16-px perimeter-removal PNG.
# This is an RGBA *pixel* hash so the source cannot silently drift across
# Pillow or zlib versions; the original archival PNG remains immutable.
APPROVED_CLEAN_RGBA_SHA256 = "3534161a2e9594802247f735c5ed70eedab0a55e40bce9fa3860a0f0d7f174f1"
INSET_PX = 16
EDGE_FEATHER_PX = 1.8
WINDOWS_SIZES = ((16,16),(24,24),(32,32),(48,48),(64,64),(128,128),(256,256))
# Only desktop derivatives are horizontally balanced; the source master is unchanged.
OPTICAL_WIDTH_SCALE = 0.976

def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def _edt_1d(values: list[int]) -> list[int]:
    """Exact O(n) squared Euclidean distance transform (parabola envelope)."""
    count = len(values)
    sites = [0] * count
    cuts = [0.0] * (count + 1)
    sites[0] = 0
    cuts[0], cuts[1] = float("-inf"), float("inf")
    k = 0
    for position in range(1, count):
        height = values[position] + position * position
        while True:
            previous = sites[k]
            crossing = (
                (height - values[previous] - previous * previous)
                / (2 * (position - previous))
            )
            if crossing > cuts[k]:
                break
            k -= 1
        k += 1
        sites[k] = position
        cuts[k], cuts[k + 1] = crossing, float("inf")
    output = [0] * count
    k = 0
    for position in range(count):
        while cuts[k + 1] < position:
            k += 1
        delta = position - sites[k]
        output[position] = delta * delta + values[sites[k]]
    return output


def approved_clean_source(image: Image.Image) -> Image.Image:
    """Reproduce the accepted edit EXACTLY: RGB unchanged, alpha-only inset.

    The 16-pixel Euclidean inset with 1.8-pixel linear feather removes the
    unwanted dark perimeter, leaving characters and inner linework intact.
    Pure Python/Pillow: avoids native SciPy/CV dependencies in both packagers.
    """
    result = image.convert("RGBA")
    width, height = result.size
    opaque = result.getchannel("A").tobytes()
    inf = 10 ** 9
    horizontal = array("I", [0]) * (width * height)
    for y in range(height):
        start = y * width
        distances = _edt_1d([
            inf if opaque[start + x] else 0 for x in range(width)
        ])
        for x, value in enumerate(distances):
            horizontal[start + x] = value
    alpha = bytearray(width * height)
    for x in range(width):
        distances = _edt_1d([
            horizontal[y * width + x] for y in range(height)
        ])
        for y, squared in enumerate(distances):
            opacity = int(
                (sqrt(squared) - INSET_PX) * 255 / EDGE_FEATHER_PX
            )
            alpha[y * width + x] = min(255, max(0, opacity))
    result.putalpha(Image.frombytes("L", result.size, bytes(alpha)))
    return result


def build(source: Path, dest: Path, *, allow_alternate_source: bool=False) -> dict:
    if not source.is_file():
        raise FileNotFoundError(f"Approved image source not present: {source}")
    actual = sha(source)
    if not allow_alternate_source and actual != SOURCE_SHA256:
        raise ValueError(
            "Artwork differs from the owner-approved transparent variant 04: "
            f"{actual}. Do not silently substitute another PyMO icon."
        )
    with Image.open(source) as opened:
        image=opened.convert("RGBA")
    if image.size != (1254,1254):
        raise ValueError(f"Unexpected original artwork size: {image.size}")
    if image.getchannel("A").getextrema()[0] >= 255:
        raise ValueError("Expected genuine transparency, not an opaque white/black square")
    dest.mkdir(parents=True,exist_ok=True)
    image = approved_clean_source(image)
    pixel_hash = hashlib.sha256(image.tobytes()).hexdigest()
    if not allow_alternate_source and pixel_hash != APPROVED_CLEAN_RGBA_SHA256:
        raise ValueError("Generated desktop artwork does not match the "
                         "approved 16px alpha-only cleanup: " + pixel_hash)
    clean_source = dest/"converter-icon-source-clean.png"
    image.save(clean_source,format="PNG",optimize=True)
    balanced=Image.new("RGBA",image.size,(0,0,0,0))
    width=round(image.width*OPTICAL_WIDTH_SCALE)
    balanced.alpha_composite(image.resize((width,image.height),Image.Resampling.LANCZOS),
                             ((image.width-width)//2,0))
    master=balanced.resize((1024,1024),Image.Resampling.LANCZOS)
    paths={
        "png":dest/"converter-icon.png",
        "png_64":dest/"converter-icon-64.png",
        "ico":dest/"converter-icon.ico",
        "icns":dest/"converter-icon.icns",
    }
    master.save(paths["png"],format="PNG",optimize=True)
    balanced.resize((64,64),Image.Resampling.LANCZOS).save(paths["png_64"],format="PNG",optimize=True)
    master.save(paths["ico"],format="ICO",sizes=list(WINDOWS_SIZES))
    master.save(paths["icns"],format="ICNS")
    with Image.open(paths["ico"]) as check:
        assert set(check.info["sizes"]) >= set(WINDOWS_SIZES)
    with Image.open(paths["icns"]) as check:
        assert check.size == (1024,1024)
        assert (512,512,2) in check.info["sizes"]
    manifest={"source_sha256":actual,
        "approved_clean_pixel_sha256": pixel_hash,
        "approved_clean_source_png_sha256": sha(clean_source),
        "clean_source": "converter-icon-source-clean.png",
        "alpha_only_inset_px": INSET_PX,
        "alpha_feather_px": EDGE_FEATHER_PX,
        "optical_width_scale":OPTICAL_WIDTH_SCALE,
        "source":"04_高清像素图标_透明外框.png (archival); clean master: 16px approved edit",
        "upstream_artwork_license":"review-required-before-public-redistribution",
        "outputs":{name:{"sha256":sha(path),"bytes":path.stat().st_size}
                   for name,path in paths.items()}}
    (dest/"icon-manifest.json").write_text(
        json.dumps(manifest,indent=2,ensure_ascii=False)+"\n",encoding="utf8")
    return manifest

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source",type=Path,required=True)
    parser.add_argument("--output",type=Path,required=True)
    parser.add_argument("--allow-alternate-source",action="store_true",
                        help="For developer exploration only, not public RC builds")
    args=parser.parse_args()
    print(json.dumps(build(args.source,args.output,
                           allow_alternate_source=args.allow_alternate_source),
                     indent=2,ensure_ascii=False))

if __name__=="__main__":
    main()

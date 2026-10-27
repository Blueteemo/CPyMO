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

SOURCE_SHA256 = "3f110860ce4e242b79147ccd7b6c6d260efd1a232fa8039b41c02677aa3f8c62"
WINDOWS_SIZES = ((16,16),(24,24),(32,32),(48,48),(64,64),(128,128),(256,256))

def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

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
    master=image.resize((1024,1024),Image.Resampling.LANCZOS)
    paths={
        "png":dest/"converter-icon.png",
        "png_64":dest/"converter-icon-64.png",
        "ico":dest/"converter-icon.ico",
        "icns":dest/"converter-icon.icns",
    }
    master.save(paths["png"],format="PNG",optimize=True)
    image.resize((64,64),Image.Resampling.LANCZOS).save(paths["png_64"],format="PNG",optimize=True)
    master.save(paths["ico"],format="ICO",sizes=list(WINDOWS_SIZES))
    master.save(paths["icns"],format="ICNS")
    with Image.open(paths["ico"]) as check:
        assert set(check.info["sizes"]) >= set(WINDOWS_SIZES)
    with Image.open(paths["icns"]) as check:
        assert check.size == (1024,1024)
        assert (512,512,2) in check.info["sizes"]
    manifest={"source_sha256":actual,
        "source":"04_高清像素图标_透明外框.png",
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

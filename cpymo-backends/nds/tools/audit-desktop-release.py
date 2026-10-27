#!/usr/bin/env python3
"""Audit the four paired CPyMO 1.2 RC distributions built in one Actions run.

Stdlib only: read the actual ZIPs, check paired ROM, corresponding FFmpeg
source, the Windows PE icon, Mac Finder icon, and required artwork attribution.
Never treat a green individual build job as cross-package acceptance.
"""
from __future__ import annotations

import argparse
import hashlib
import io
import json
import os
from pathlib import Path
import plistlib
import struct
import zipfile

APPROVED_ROM_SHA256 = "7759a5db97d744aa3e3014e24550cd6b23d0c758735c60cfd835f0b92e51ae31"
APPROVED_ICON_SOURCE_SHA256 = "3f110860ce4e242b79147ccd7b6c6d260efd1a232fa8039b41c02677aa3f8c62"
APPROVED_FFMPEG_SOURCE_SHA256 = "de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f"
VERSION = "1.2.0"


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_zip(path: Path) -> zipfile.ZipFile:
    z = zipfile.ZipFile(path)
    damaged = z.testzip()
    if damaged is not None:
        raise ValueError(f"Corrupt member in {path}: {damaged}")
    return z


def member(z: zipfile.ZipFile, suffix: str) -> str:
    matches = [p for p in z.namelist() if p.endswith("/" + suffix)]
    if len(matches) != 1:
        raise ValueError(f"Expected one {suffix}, got {matches}")
    return matches[0]


def ensure_pe_icon(payload: bytes) -> dict:
    """Check the actual .exe resources, not just PyInstaller's --icon CLI."""
    u16 = lambda i: struct.unpack_from("<H", payload, i)[0]
    u32 = lambda i: struct.unpack_from("<I", payload, i)[0]
    assert payload[:2] == b"MZ"
    pe = u32(0x3c)
    assert payload[pe:pe + 4] == b"PE\x00\x00"
    coff = pe + 4
    optional = coff + 20
    nsec, opt_size = u16(coff + 2), u16(coff + 16)
    dir_base = optional + (112 if u16(optional) == 0x20b else 96)
    resource_rva = u32(dir_base + 16)
    sections = []
    for j in range(nsec):
        o = optional + opt_size + j * 40
        sections.append((u32(o+12), u32(o+8), u32(o+20), u32(o+16)))

    def offset(rva):
        for virtual, size, raw, raw_size in sections:
            if virtual <= rva < virtual + max(size, raw_size):
                return raw + rva - virtual
        raise ValueError(f"PE resource RVA out of bounds: {rva:#x}")

    base = offset(resource_rva)

    def entries(rel):
        o = base + rel
        count = u16(o + 12) + u16(o + 14)
        return [(u32(o + 16 + i * 8), u32(o + 20 + i * 8))
                for i in range(count)]

    root = entries(0)
    def child(typ):
        candidates = [rel for key, rel in root if key == typ and rel & 0x80000000]
        assert len(candidates) == 1, f"Missing PE icon resource {typ}"
        return entries(candidates[0] & 0x7fffffff)

    images = child(3)   # RT_ICON
    groups = child(14)  # RT_GROUP_ICON
    assert len(images) >= 7 and len(groups) == 1
    lang_entries = entries(groups[0][1] & 0x7fffffff)
    assert lang_entries
    _, leaf = lang_entries[0]
    assert not leaf & 0x80000000
    data_rva = u32(base + leaf)
    size = u32(base + leaf + 4)
    data = payload[offset(data_rva):offset(data_rva)+size]
    reserved, kind, count = struct.unpack_from("<HHH", data)
    assert (reserved, kind, count) == (0, 1, 7)
    sizes = set()
    for i in range(count):
        w, h, _, _, _, bpp, _, _ = struct.unpack_from("<BBBBHHIH", data, 6+i*14)
        assert bpp == 32
        sizes.add((w or 256, h or 256))
    assert {(16,16), (24,24), (32,32), (48,48), (64,64), (128,128), (256,256)} <= sizes
    return {"PE_group_icon": "PASS", "sizes": sorted(sizes)}


def main():
    ap = argparse.ArgumentParser()
    for name in ("rom", "windows", "macos", "sources"):
        ap.add_argument("--" + name, type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    args = ap.parse_args()
    paths = {name: getattr(args, name) for name in ("rom", "windows", "macos", "sources")}
    for path in paths.values():
        if not path.is_file():
            ap.error(f"Missing paired distribution {path}")
    rom = args.rom.read_bytes()
    assert len(rom) == 924672 and digest(rom) == APPROVED_ROM_SHA256
    report = {"status": "RC verified, not a published GitHub Release",
              "source_commit": os.environ.get("GITHUB_SHA", "unknown"),
              "workflow_run": os.environ.get("GITHUB_RUN_ID", "local"),
              "rom_sha256": digest(rom), "distributions": {}}
    zip_reports = {}
    ffmpeg_sources = []
    for kind in ("windows", "macos"):
        with read_zip(paths[kind]) as z:
            info = {"bytes": paths[kind].stat().st_size,
                    "sha256": digest(paths[kind].read_bytes())}
            roms = [p for p in z.namelist() if p.endswith("/cpymo-nds.nds")
                    and z.getinfo(p).file_size == 924672]
            assert len(roms) == 2, (kind, roms)
            assert all(digest(z.read(n)) == digest(rom) for n in roms)
            metadata = json.loads(z.read(member(z, "VERSION.json")))
            assert metadata["version"] == VERSION
            assert metadata["rom_sha256"] == digest(rom)
            assert metadata["desktop_icon"] == "approved-PyMO-04"
            for label in ("图标来源与署名.txt", "PYMO-ORIGINAL-MIT-LICENSE.txt"):
                contents = z.read(member(z, label))
                assert len(contents) > 100 and b"pymo" in contents.lower(), label
            src = z.read(member(z, "FFmpeg-Corresponding-Source/ffmpeg-7.1.5.tar.xz"))
            assert digest(src) == APPROVED_FFMPEG_SOURCE_SHA256
            ffmpeg_sources.append(digest(src))
            conf = z.read(member(z, "FFmpeg-Corresponding-Source/FFMPEG-BUILDCONF.txt")).decode()
            assert "--disable-gpl" in conf and "--disable-nonfree" in conf
            assert "--enable-gpl" not in conf and "--enable-nonfree" not in conf
            info["rom_matches"] = True
            info["pymo_icon_license_and_attribution"] = "present"
            info["ffmpeg_corresponding_source_sha256"] = digest(src)
            if kind == "windows":
                exe = z.read(member(z, "Converter/Converter.exe"))
                info["windows_icon"] = ensure_pe_icon(exe)
            else:
                plist = plistlib.loads(z.read(member(z, "CPyMO-NDS-Converter.app/Contents/Info.plist")))
                assert plist.get("CFBundleVersion") == VERSION
                assert plist.get("CFBundleShortVersionString") == VERSION
                assert plist.get("CFBundleIconFile") == "converter-icon.icns"
                icns = z.read(member(z, "Contents/Resources/converter-icon.icns"))
                assert icns[:4] == b"icns" and len(icns) > 100000
                assert metadata.get("macos_architecture") == "arm64"
                assert metadata.get("rosetta_required") is False
                info["macos_icon"] = {"sha256": digest(icns), "plist": "MATCH"}
                info["native_arch"] = "arm64"
            report["distributions"][kind] = info
    assert len(set(ffmpeg_sources)) == 1
    with read_zip(paths["sources"]) as z:
        source_icon = z.read(member(z, "cpymo-tool/nds-converter/assets/converter-icon-source.png"))
        assert digest(source_icon) == APPROVED_ICON_SOURCE_SHA256
        for name in ("ICON-ATTRIBUTION.txt", "PYMO-ORIGINAL-MIT-LICENSE.txt"):
            assert z.getinfo(member(z, "cpymo-tool/nds-converter/assets/" + name)).file_size > 100
        assert z.getinfo(member(z, "LICENSE")).file_size > 100
    report["distributions"]["sources"] = {"bytes": paths["sources"].stat().st_size,
                                           "sha256": digest(paths["sources"].read_bytes()),
                                           "approved_icon_source_sha256": digest(source_icon)}
    report["distributions"]["rom"] = {"bytes": len(rom), "sha256": digest(rom)}
    args.output.mkdir(parents=True, exist_ok=True)
    output = args.output / "CPyMO-for-NDS-1.2.0-RC-SHA256SUMS.txt"
    output.write_text("".join(f"{digest(paths[name].read_bytes())}  {paths[name].name}\n"
                              for name in ("rom","windows","macos","sources")), encoding="ascii")
    (args.output / "RC-PAIRED-AUDIT.json").write_text(
        json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()

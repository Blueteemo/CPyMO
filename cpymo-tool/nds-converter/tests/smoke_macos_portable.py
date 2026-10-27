"""macOS frozen .app smoke: real MP4->NVL1, glyph fallback, paired ROM.

Run on an actual macOS CI runner after build_portable_macos.py.
Synthetic assets only; does not attest to Finder/Gatekeeper acceptance.
"""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import sys
import tempfile

from test_converter import fixture, hashes


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser()
    p.add_argument("--portable", type=Path, required=True)
    p.add_argument("--ffmpeg", type=Path, required=True)
    a=p.parse_args()
    root=a.portable.resolve()
    app=root/"CPyMO-NDS-Converter.app"
    exe=app/"Contents/MacOS/CPyMO-NDS-Converter"
    assert exe.is_file(), "Frozen .app executable is missing"
    rom=root/"cpymo-nds.nds"
    assert rom.is_file()
    assert digest(rom)==json.loads((root/"VERSION.json").read_text())["rom_sha256"]
    with tempfile.TemporaryDirectory(prefix="cpymo-macos-smoke-") as temp:
        base=Path(temp);source=base/"source";fixture(source,"s60v3")
        movie=source/"video/omitted.mp4"
        cmd=[str(a.ffmpeg),"-nostdin","-hide_banner","-loglevel","error",
             "-f","lavfi","-i","testsrc2=duration=1.2:size=320x240:rate=12",
             "-f","lavfi","-i","sine=frequency=440:duration=1.2:sample_rate=44100",
             "-c:v","mpeg4","-q:v","7","-c:a","aac","-y",str(movie)]
        subprocess.run(cmd,check=True,timeout=90)
        original=hashes(source)
        result=base/"result.json";log=base/"log.txt";out=base/"converted"
        job={"action":"convert","source":str(source),"output":str(out),
             "result":str(result),"log":str(log)}
        cfg=base/"job.json";cfg.write_text(json.dumps(job),encoding="utf8")
        done=subprocess.run([str(exe),"--job",str(cfg)],
                            capture_output=True,text=True,timeout=180)
        assert result.is_file(),f"Frozen conversion failed {done.returncode}: {done.stderr[-1200:]}"
        report=json.loads(result.read_text(encoding="utf8"))
        assert done.returncode==0 and report["ok"], (report,log.read_text(errors="replace")[-1400:])
        assert report["videos"][0]["format"]=="NVL1",report
        assert report["missing_policy"]=="box",report
        assert hashes(source)==original,"Source was modified"
        assert digest(out/"cpymo-nds.nds")==digest(rom),"Bundled ROM mismatch"
        assert (Path(report["game_directory"])/"nds-cache/video").is_dir()
        print(json.dumps({"mac_frozen_worker":"PASS","default_video":"NVL1",
                          "missing_glyph_policy":"box","paired_rom":"MATCH",
                          "source_integrity":"PASS"},ensure_ascii=False))


if __name__=="__main__":
    main()

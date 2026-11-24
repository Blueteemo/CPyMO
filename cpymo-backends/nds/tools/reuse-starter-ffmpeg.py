"""Extract a release-pinned FFmpeg build and its corresponding source."""

import argparse
import hashlib
from pathlib import Path
import stat
import zipfile


BASELINES = {
    "windows": (
        "2fc7191210c15ccac619239b29ea839677f5df81045758bdf4f8676581b409b4",
        "9e97992e9724aa6329663f18d72cdce5b809fbc4e0bb3fb9a7c88e6653ccd7a3",
        "Converter/_internal/ffmpeg.exe",
    ),
    "macos": (
        "1b06441f1399ee4e6508eecf82408f823bce5345218dee23c3b8d6a2ed4a822f",
        "4f6fbf1ce19a2873da38422cd60530d9a58e52dd4ece6494c5b9b6418db679b6",
        "Contents/Frameworks/ffmpeg",
    ),
}
SOURCE_SHA256 = "de668509caf9e35e3cd162473441fdb29538c6d96ed080292b3cf9e6fc5d558f"
SOURCE_DIR = "FFmpeg-Corresponding-Source"


def extract(starter: Path, platform: str, output: Path) -> None:
    starter_hash, ffmpeg_hash, executable_suffix = BASELINES[platform]
    with starter.open("rb") as file:
        digest = hashlib.sha256()
        for block in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(block)
    if digest.hexdigest() != starter_hash:
        raise ValueError("The prior official Starter does not match its pinned checksum")

    with zipfile.ZipFile(starter) as archive:
        items = [item for item in archive.infolist() if not item.is_dir()
                 and not item.filename.startswith("__MACOSX/")]
        executable, = [item for item in items if item.filename.endswith("/" + executable_suffix)]
        source_files = {}
        for item in items:
            if f"/{SOURCE_DIR}/" not in item.filename:
                continue
            name = item.filename.split(f"/{SOURCE_DIR}/", 1)[1]
            if not name or name in {".", ".."} or "/" in name or "\\" in name or ":" in name:
                raise ValueError(f"Unexpected FFmpeg source member: {name}")
            if name in source_files or stat.S_ISLNK(item.external_attr >> 16):
                raise ValueError(f"Ambiguous FFmpeg source member: {name}")
            source_files[name] = archive.read(item)
        ffmpeg = archive.read(executable)

    if hashlib.sha256(ffmpeg).hexdigest() != ffmpeg_hash:
        raise ValueError("The prior FFmpeg binary identity differs")
    if hashlib.sha256(source_files["ffmpeg-7.1.5.tar.xz"]).hexdigest() != SOURCE_SHA256:
        raise ValueError("The FFmpeg corresponding source archive identity differs")
    config = source_files["FFMPEG-BUILDCONF.txt"]
    for option in (b"gpl", b"nonfree"):
        if b"--disable-" + option not in config or b"--enable-" + option in config:
            raise ValueError("Unexpected FFmpeg redistribution flags")

    dest = output / SOURCE_DIR
    dest.mkdir(parents=True)
    for name, data in source_files.items():
        (dest / name).write_bytes(data)
    binary = output / executable_suffix.rsplit("/", 1)[1]
    binary.write_bytes(ffmpeg)
    binary.chmod(0o755)
    print(f"{platform} FFmpeg SHA-256 {ffmpeg_hash}; matching corresponding source retained")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("platform", choices=BASELINES)
    parser.add_argument("starter", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    extract(args.starter, args.platform, args.output)

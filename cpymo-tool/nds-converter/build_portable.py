"""Build a paired Windows ROM/converter distribution. MIT; Python 3.10+."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
BACKEND = ROOT.parents[1] / 'cpymo-backends/nds'

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ffmpeg', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--rebuild', action='store_true', help='Reuse an owned portable-build directory')
    args = parser.parse_args()
    if os.name != 'nt':
        parser.error('Build the Windows portable executable on Windows.')
    ffmpeg = args.ffmpeg.resolve()
    output = args.output.resolve()
    rom = BACKEND / 'engine.nds'
    owned = (output/'build/Converter.spec').is_file() and (output/'helper-build.log').is_file()
    if (output.exists() and not (args.rebuild and owned)) or not ffmpeg.is_file() or not rom.is_file():
        parser.error('Use a new output directory and existing FFmpeg/engine.nds files.')
    release = json.loads((BACKEND/'release.json').read_text(encoding='utf8'))
    output.mkdir(parents=True, exist_ok=args.rebuild)
    build = output/'build'
    resources = build/'resources'
    resources.mkdir(parents=True, exist_ok=True)
    paired_rom = resources/'cpymo-nds.nds'
    shutil.copy2(rom, paired_rom)
    command = [sys.executable, '-m', 'PyInstaller', '--noconfirm', '--clean',
               '--onedir', '--windowed', '--name', 'Converter',
               '--distpath', str(output/'app'), '--workpath', str(build),
               '--specpath', str(build), '--add-data', f'{ROOT / "fonts"};fonts',
               '--add-data', f'{BACKEND / "release.json"};.',
               '--add-data', f'{paired_rom};.', '--add-binary', f'{ffmpeg};.',
               '--exclude-module', 'numpy', '--exclude-module', 'scipy',
               '--exclude-module', 'matplotlib', '--exclude-module', 'pytest',
               str(ROOT/'app.py')]
    with (output/'helper-build.log').open('w', encoding='utf8') as log:
        subprocess.run(command, check=True, stdout=log, stderr=subprocess.STDOUT)
    shutil.copy2(rom, output/'cpymo-nds.nds')
    digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    assert digest(rom) == digest(output/'app/Converter/_internal/cpymo-nds.nds')
    release.update(rom_sha256=digest(rom), ffmpeg_sha256=digest(ffmpeg))
    (output/'VERSION.json').write_text(json.dumps(release, indent=2)+'\n', encoding='utf8')
    (output/'BUILD-COMMAND.json').write_text(json.dumps(command, indent=2), encoding='utf8')
    print(json.dumps(release, indent=2))

if __name__ == '__main__':
    main()

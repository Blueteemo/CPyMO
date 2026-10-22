"""Original CPyMO NDS NVL1/NVA1 encoder; no external proprietary codec tables.

FFmpeg supplies normalized RGB and PCM. We only package bounded 256x192
RGB555 LZ4/XOR pictures plus 22,050-Hz mono samples in the format understood
by cpymo-backends/nds/source/movie-lz4-r7.h. The source is never modified.
"""
from __future__ import annotations
from pathlib import Path
import hashlib
import os
import shutil
import struct
import subprocess
import tempfile
import zlib

import lz4.block
import lossless

WIDTH, HEIGHT, RATE = 256, 192, 22050
RAW_RGB = WIDTH * HEIGHT * 3
RAW_FRAME = WIDTH * HEIGHT * 2
VIDEO_SUFFIXES = frozenset({'.mp4', '.avi', '.mpg', '.mpeg', '.mkv', '.webm',
                            '.3gp', '.wmv', '.mov', '.ogv', '.flv', '.m4v'})


def _rgb555(rgb: bytes) -> bytes:
    if len(rgb) != RAW_RGB:
        raise ValueError('Truncated RGB24 movie frame')
    out = bytearray(RAW_FRAME)
    j = 0
    for i in range(0, RAW_RGB, 3):
        pixel = 0x8000 | (rgb[i] >> 3) | ((rgb[i+1] >> 3) << 5) | ((rgb[i+2] >> 3) << 10)
        out[j] = pixel & 255
        out[j+1] = pixel >> 8
        j += 2
    return bytes(out)


def encode(source, destination, ffmpeg='ffmpeg', *, fps=12, audio='pcm'):
    """Write player-compatible .nvl to a new file in the caller's staging area."""
    source, destination = Path(source).resolve(), Path(destination).resolve()
    if not source.is_file() or source.suffix.lower() not in VIDEO_SUFFIXES:
        raise ValueError('Video input must be a supported existing media file')
    if source == destination or destination.exists():
        raise ValueError('Video output must be a new file distinct from source')
    if audio not in ('pcm', 'nar') or fps not in (6, 8, 12, 15, 20, 24):
        raise ValueError('Unsupported NDS movie FPS/audio option')
    ffmpeg = str(ffmpeg)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='nvl-', dir=destination.parent) as temp:
        temp = Path(temp)
        video_stream = temp/'frames.bin'
        pcm = temp/'sound.pcm'
        # Decode one RGB24 frame at a time; memory independent of duration.
        filterchain = (f'fps={fps},scale={WIDTH}:{HEIGHT}:'
                       'force_original_aspect_ratio=decrease:flags=bicubic,'
                       f'pad={WIDTH}:{HEIGHT}:(ow-iw)/2:(oh-ih)/2:black,format=rgb24')
        command = [ffmpeg, '-nostdin', '-hide_banner', '-loglevel', 'error',
                   '-i', str(source), '-map', '0:v:0', '-an',
                   '-vf', filterchain, '-f', 'rawvideo', '-pix_fmt', 'rgb24', 'pipe:1']
        frames = 0
        previous = None
        with video_stream.open('wb') as out:
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                       creationflags=0x08000000 if os.name == 'nt' else 0)
            try:
                while True:
                    raw = process.stdout.read(RAW_RGB)
                    if not raw:
                        break
                    if len(raw) != RAW_RGB:
                        raise ValueError('Truncated source video frame')
                    if frames >= fps*600:
                        raise ValueError('NDS video longer than 600 seconds')
                    frame = _rgb555(raw)
                    key = previous is None or frames % (fps*4) == 0
                    delta = frame if key else bytes(a ^ b for a,b in zip(frame,previous))
                    packed = lz4.block.compress(delta, mode='high_compression',
                                                compression=6, store_size=False)
                    use_packed = len(packed) < RAW_FRAME
                    payload = packed if use_packed else delta
                    out.write(struct.pack('<III', len(payload),
                              (1 if use_packed else 0) | (0 if key else 2),
                              zlib.adler32(frame)))
                    out.write(payload)
                    previous = frame
                    frames += 1
                stderr = process.stderr.read()
                if process.wait() != 0:
                    raise ValueError('FFmpeg video decoding failed: '+
                                     stderr.decode('utf-8','replace')[-1000:])
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait()
                process.stdout.close()
                process.stderr.close()
        if not frames:
            raise ValueError('Video contains no decodable frames')
        samples = frames*RATE//fps
        # Force resampling BEFORE atrim; otherwise source rate (e.g. 44.1k)
        # can cause the output to have only half the expected sample count.
        audio_command = [ffmpeg, '-nostdin', '-hide_banner', '-loglevel', 'error',
                         '-i', str(source), '-map', '0:a:0?', '-vn', '-ac', '1',
                         '-ar', str(RATE), '-af',
                         f'aresample={RATE},apad,atrim=end_sample={samples}',
                         '-f', 's16le', str(pcm)]
        result = subprocess.run(audio_command, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                creationflags=0x08000000 if os.name == 'nt' else 0)
        if result.returncode:
            # FFmpeg spelling differs by version. Accept only the explicit
            # "no audio stream" case; real decoding errors must fail the job.
            if (b'Output file does not contain any stream' not in result.stderr
                    and b'Output file #0 does not contain any stream' not in result.stderr):
                raise ValueError('FFmpeg audio decoding failed: '+
                                 result.stderr.decode('utf-8','replace')[-1000:])
            with pcm.open('wb') as out:
                chunk = b'\x00'*8192
                remaining = samples*2
                while remaining:
                    n = min(len(chunk),remaining)
                    out.write(chunk[:n])
                    remaining -= n
        if pcm.stat().st_size != samples*2:
            raise ValueError('Audio sample count did not match video playback clock')
        data = pcm
        if audio == 'nar':
            data = temp/'sound.nar'
            audit = lossless.encode_audio(pcm, data)
            if audit['samples'] != samples:
                raise ValueError('NAR2 sample count changed')
        start = 32 + video_stream.stat().st_size
        if start >= 512*1024*1024:
            raise ValueError('NDS video packets exceed 512 MiB limit')
        magic = b'NVA1' if audio=='nar' else b'NVL1'
        try:
            with destination.open('xb') as out:
                out.write(struct.pack('<4s7I',magic,WIDTH,HEIGHT,
                                      fps,frames,RATE,samples,start))
                with video_stream.open('rb') as inp:
                    shutil.copyfileobj(inp,out,1024*1024)
                with data.open('rb') as inp:
                    shutil.copyfileobj(inp,out,1024*1024)
        except BaseException:
            destination.unlink(missing_ok=True)
            raise
    # Hash in chunks, including for long full-length clips.
    h=hashlib.sha256()
    with destination.open('rb') as inp:
        for block in iter(lambda:inp.read(1024*1024),b''):
            h.update(block)
    return {'format':magic.decode('ascii'),'frames':frames,'fps':fps,
            'duration_seconds':frames/fps,'audio_samples':samples,'audio_rate':RATE,
            'width':WIDTH,'height':HEIGHT,'bytes':destination.stat().st_size,
            'sha256':h.hexdigest(),'has_audio':result.returncode==0}

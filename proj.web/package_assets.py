#!/usr/bin/env python3
"""Stage original assets and decode legacy music to browser-compatible PCM WAV."""
import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--ffmpeg', required=True)
args = parser.parse_args()
if args.output.resolve() == args.source.resolve() or args.output.resolve() in args.source.resolve().parents or args.source.resolve() in args.output.resolve().parents:
    raise ValueError('Generated output must not replace the original resource tree')
if args.output.exists():
    shutil.rmtree(args.output)
args.output.mkdir(parents=True, exist_ok=True)
shutil.copytree(args.source, args.output, dirs_exist_ok=True,
                ignore=lambda directory, names: ['song.mp3'] if Path(directory) == args.source else [])
audio = args.output / 'browser-audio'
audio.mkdir(exist_ok=True)
subprocess.run([args.ffmpeg, '-nostdin', '-v', 'error', '-y', '-i',
    str(args.source / 'song.mp3'), '-map_metadata', '-1', '-fflags', '+bitexact',
    '-flags:a', '+bitexact', '-c:a', 'pcm_s16le', str(audio / 'song.wav')], check=True)
manifest = {'music_alias': {'song.mp3': 'browser-audio/song.wav'},
    'original_music_sha256': hashlib.sha256((args.source / 'song.mp3').read_bytes()).hexdigest(),
    'converted_music_sha256': hashlib.sha256((audio / 'song.wav').read_bytes()).hexdigest()}
(args.output / 'web-assets.json').write_text(json.dumps(manifest, indent=2) + '\n')
(args.output.parent / 'assets.stamp').touch()

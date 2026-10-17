#!/usr/bin/env python3
"""Stage assets and normalize browser audio without changing source recordings."""
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
def ignore_assets(directory, names):
    folder = Path(directory)
    ignored = [name for name in names if name == '.DS_Store' or
               (Path(name).suffix.lower() in {'.wav','.mp3','.ogg'} and Path(name).stem.endswith('_old'))]
    if folder == args.source:
        ignored.append('song.mp3')
    if folder == args.source / 'web-audio':
        ignored.extend(name for name in names if Path(name).suffix.lower() in {'.mp3','.ogg'})
    return ignored

shutil.copytree(args.source, args.output, dirs_exist_ok=True, ignore=ignore_assets)
# Source replacements may be 24-bit/float WAV, MP3, or Ogg. Stage PCM WAVs for
# predictable browser decoding. A same-stem WAV takes priority over MP3/Ogg.
custom_audio = []
for source in sorted((args.source / 'web-audio').glob('*')):
    if source.suffix.lower() not in {'.wav','.mp3','.ogg'} or source.stem.endswith('_old'):
        continue
    if source.suffix.lower() != '.wav' and source.with_suffix('.wav').exists():
        continue
    destination = args.output / 'web-audio' / (source.stem + '.wav')
    filters = []
    if source.stem == 'crowd':
        # Quick entry and natural end, independent of the recording's duration.
        filters = ['-af', 'afade=t=in:st=0:d=0.2,areverse,afade=t=in:st=0:d=0.5,areverse']
    subprocess.run([args.ffmpeg, '-nostdin', '-v', 'error', '-y', '-i', str(source),
        '-map_metadata', '-1', '-ar', '44100', '-c:a', 'pcm_s16le', *filters, str(destination)], check=True)
    custom_audio.append(str(destination.relative_to(args.output)))
audio = args.output / 'browser-audio'
audio.mkdir(exist_ok=True)
subprocess.run([args.ffmpeg, '-nostdin', '-v', 'error', '-y', '-i',
    str(args.source / 'song.mp3'), '-map_metadata', '-1', '-fflags', '+bitexact',
    '-flags:a', '+bitexact', '-c:a', 'pcm_s16le', str(audio / 'song.wav')], check=True)
manifest = {'custom_audio': custom_audio, 'music_alias': {'song.mp3': 'browser-audio/song.wav'},
    'original_music_sha256': hashlib.sha256((args.source / 'song.mp3').read_bytes()).hexdigest(),
    'converted_music_sha256': hashlib.sha256((audio / 'song.wav').read_bytes()).hexdigest()}
(args.output / 'web-assets.json').write_text(json.dumps(manifest, indent=2) + '\n')
(args.output.parent / 'assets.stamp').touch()

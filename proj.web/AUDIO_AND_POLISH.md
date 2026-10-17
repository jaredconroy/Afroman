# Browser audio and gameplay additions

All additions live in the separate Afroman-web checkout. The Mac archive is unchanged.

## Replace the placeholder audio

The seven files in `Resources/web-audio/` are original synthesized placeholders,
created by `proj.web/generate_placeholder_audio.py`; no third-party recordings or
music were downloaded. Music uses pulse/triangle voices and noise percussion for
an 8-bit sound, stored as ordinary 16-bit PCM WAV for dependable browser decoding.

| File | Used for |
| --- | --- |
| `level1.wav` | Looping sidescroller music, 120 BPM, 16 seconds |
| `level3.wav` | Looping maze music, 100 BPM, 19.2 seconds |
| `jump.wav` | Jump/bounce on the upper arrow's grounded jump |
| `land.wav` | Downward landing impact |
| `flush.wav` | First contact with the far-left upper-floor wall |
| `win.wav` | Level 1/3 completion |
| `lose.wav` | Level 1/3 failure or timeout |

Replace any of these files with your own audio, keeping the filename and using
actual PCM WAV data (renaming an MP3 to `.wav` does not convert it). For example,
from the project root:

```sh
ffmpeg -i "/absolute/path/to/your-music.mp3" -map_metadata -1 \
  -ar 44100 -ac 2 -c:a pcm_s16le Resources/web-audio/level1.wav
./proj.web/build.sh Debug
./proj.web/serve.sh
```

Reload the browser after rebuilding. The asset-packaging step automatically copies
and embeds replacements. Use loop-ready tracks without encoder silence at the
ends; keep music quieter than effects. Current placeholders peak well below full
scale. Mono or stereo WAV is accepted; short loops keep the download small.
`generate_placeholder_audio.py` is **manual only**: rerunning it overwrites all
seven files with defaults. Builds never regenerate or overwrite your source audio.
The original level 2 song and drum recordings remain unchanged.

Music pauses with pause/end menus, resumes with RESUME, restarts with RESTART,
and stops on SELECT LEVEL or EXIT. Browser audio is still unlocked by Play Afroman.

## Gameplay

- Level 2 keeps its original three-button results menu. All seven drum keys
  remain playable behind it without scoring or timing penalties until you choose
  restart, select level, or exit. There is no separate free-play button or screen.
  The score panel sits right of center so the drum labels and key squares remain
  visible, and its full-screen dimming region is cropped away.
- The controls intro shows a five-second PRACTICE countdown and fades it with
  the controls. It follows the existing audio/chart clock, including calibration
  and pause/resume; the initial scene transition is included in the intro lead-in.
  Neither song playback nor note timing has shifted. The pause artwork and its
  buttons move six pixels right to align the AFROMAN logo with the controls art.
- On entering a ladder climb or sewer descent, the player's visual center snaps
  to the center of the connected horizontal trigger region. Single-tile ladders
  and seven-tile sewer entrances share their authored center; horizontal motion
  remains locked during climbing.
- The maze leaves a persistent 7-pixel-wide blue trail behind its marker, using
  RGB (15,117,188) sampled from `level3sprite.png`. Restart clears the trail.
- Both countdowns use elapsed seconds in the browser. At 10 seconds the label turns
  bright red; at each of the final five seconds it pulses from 120% to normal scale
  over 350 ms. Pausing freezes both countdown and pulse phase.

## Verification

```sh
clang++ -std=c++11 -IClasses proj.web/tests/countdown.test.cpp -o /tmp/afroman-countdown-test
/tmp/afroman-countdown-test
.tools/emsdk/node/24.19.0_64bit/bin/node proj.web/tests/audio.test.cjs
./proj.web/build.sh Debug
```

Countdown checks cover 15/30/60/144 Hz, warning thresholds, pulse phase, dropped
frames, and expiry without going negative. Audio transport regression checks cover
pause/resume and music/effect isolation. Browser listening and ladder feel remain
subjective playtesting items.

Safari regression check: the intro countdown stayed synchronized through pause
and resume; pause/control AFROMAN logos aligned. The final score panel left all
drum labels visible at full brightness, retained three original buttons with
clear logo spacing, and accepted all seven drum keys without altering the final
80/93 score during the check.

Score-panel spacing: the panel/menu group is 40 pixels left and 12 pixels down
from the previous placement. Its left edge is at canvas x=293, leaving a gutter
after the longest TOM label equal to the labels' 17-pixel outer left margin. The
score artwork's alpha-128 dim strips are discarded, and its gray backing ends
at source y=560 so it clears the BASS rail after lowering the panel. Original
SCORE and AFROMAN lettering is retained; source PNGs are unchanged.

Safari verified the top-right score and BASS rail remain undimmed and clear of
the gray backing; SCORE/AFROMAN lettering and all three buttons render correctly.

# Browser audio and gameplay additions

All additions live in the separate Afroman-web checkout. The Mac archive is unchanged.

## Replace the placeholder audio

The gameplay files in `Resources/web-audio/` are original synthesized placeholders,
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

## Title sounds

Three additional original synthesized placeholders are generated independently by
`proj.web/generate_title_audio.py` (manual only; overwrites these three files):

| File | Cue |
| --- | --- |
| `spotlight.wav` | Left spotlight at intro frame 2 (2 s), right at frame 3 (3 s), with slight stereo/pitch variation |
| `neon-plink.wav` | One glass-like plink when the neon sequence first appears (4 s) |
| `neon-buzz.wav` | Continuous looping 60 Hz electrical hum with harmonics |

The eleven neon frames each last 140 ms. Brightness measured from the artwork
peaks at frame 0 and reaches its minimum at frames 5–6. The buzz's per-voice gain
follows those same displayed frame callbacks, from 100% to 25% and back, with
25 ms smoothing to prevent clicks. It does not change music or other effect gains.
Both sound and artwork therefore pause when the browser suspends the game.

Opening level selection immediately stops all title-owned sounds and prevents
further cues behind that menu. Leaving the title scene stops its owned voices too;
returning directly to level selection does not replay the cues. EXIT to the title
starts a fresh sequence. Replace any of these WAV files and rebuild using the
same workflow above; a buzz replacement should be a short seamless steady loop,
since the animation applies the volume envelope during playback.

Safari check: the opening reached the animated neon stage with active audio;
Enter opened level selection and cleared Safari's active-audio indicator. The
final subjective timbre and loudness remain adjustable by replacing the WAVs.

## Refreshed user sounds and guitar/crowd cue

The replacement flush, landing, spotlight, neon-plink, and buzz recordings are
kept exactly as supplied in `Resources/web-audio/`. `guitar.mp3` is user supplied;
the asset packager converts it to staged `web-audio/guitar.wav`. Guitar plays once
at neon turn-on at 70% gain, alongside the existing plink and animated buzz.
`crowd.wav` is an original synthesized distant crowd-like placeholder (filtered
noise plus layered cheering tones), mixed at 35% gain and looped from neon
turn-on until Enter. Replace it with your own crowd recording if desired.
`generate_crowd_audio.py` restores only this placeholder.

Custom `.wav`, `.mp3`, and `.ogg` files are now staged as 44.1 kHz, 16-bit PCM WAV;
use the same basename when replacing a cue. A same-basename source WAV takes
priority over MP3/Ogg. Packaging excludes `_old` audio backups and `.DS_Store`,
while leaving them untouched on disk. The existing scripts that restore original
placeholder sounds are never run by a normal build.

## Bathroom lighting, victory guitar, and crowd ending

The latest user crowd and neon-plink replacements are packaged without modifying
source files. The title crowd now plays **once** (no loop). Packaging applies a
200 ms fade-in and 500 ms fade-out to its staged WAV, independent of duration;
Enter still immediately stops title-owned sounds. Level 1 also plays guitar.wav
alongside its victory cue when the destination is reached with time remaining.

The level 1 bathroom interior is taken from the authored upper-floor tilemap:
world x=416..736, y=1024..1600 at the map's 2× scale. A ceiling fixture and warm
light switch on as the player's center enters, and off on exit. The room shade
transitions over 120 ms; a 16-pixel doorway hysteresis prevents repeated clicks
while straddling the boundary. `web-audio/lightswitch.wav` is an original
synthesized click, replaceable like the other cues. Its optional manual generator
is `proj.web/generate_switch_audio.py`; builds do not run it.

Verification: browser asset decode/startup succeeded; the staged 25.875-second
crowd recording starts and ends at silence. The build passes. Bathroom entry/exit
visual verification was interrupted by a browser-control timeout and remains a
play check; the trigger was located from the actual TMX artwork and collision map.

## Latest mix adjustments

Landing plays at 75% gain. All three title spotlight cues (left, right, neon)
use pitch 1.0 and gain 0.8; the neon cue replaces the plink. Title guitar remains,
but level 1 victory no longer plays guitar. Crowd gain is 45%, retaining the
one-shot fades. The lit bathroom has no remaining dark shade, a brighter warm
cone (22% alpha), and a larger/brighter fixture glow (radius 32, 28% alpha).
Source sound files remain unchanged by these playback-gain adjustments.

## Brighter bathroom and trash-bin interaction

Normal landing gain is now 0.5625 (another 25% reduction from 0.75). The guitar
starts at neon turn-on; the third spotlight cue follows 120 ms later to account
for the guitar recording's measured ~94 ms lead-in. Spotlight pitch remains 1.0.
The bathroom now uses only full-room brightness, its small fixture, and a white
window-pane overlay; the diagonal cone/glow has been removed. The overlay matches
the actual pane at world (406,1206), size 146×212, covering its top edge.

The first exterior bin is authored at TMX columns 175–179, body rows 12–17,
with its lid on foreground row 11. Its art is displayed as one shaking sprite;
the original collision tiles stay fixed and keep their GIDs. A side bump or
landing on the bin triggers `trash-metal.wav` and `trash-slosh.wav`, a short
5-degree/6-pixel shake, and four small scraps that fall and fade in 550 ms.
A 400 ms cooldown avoids rapid retriggering, and sustained side contact triggers
only once. A bin-top landing substitutes these sounds for the ordinary land.wav.

Both new trash sounds are original synthesized placeholders; replace their WAVs
and rebuild as usual. `generate_trash_audio.py` restores only these two defaults.

Verification: browser build passes and both staged trash WAVs are valid,
unclipped PCM. Bin visual/contact playtesting was interrupted by browser
interaction and remains a gameplay check; collision coordinates were verified
against the actual body/lid tile GIDs.

## Contact and preview refresh follow-up

Side contact is now checked against the bin bounds before legacy tile resolution,
rather than depending on which neighboring collision tile is resolved first.
Landing detection uses the player foot position and bin-top bounds.
Build passes; Safari automation timed out before contact animation could be
visually confirmed in this follow-up.

`build.sh` fingerprints JS/WASM/data and stamps a shared build ID into the HTML
and all runtime asset URLs. Diagnostics displays this ID. `serve.sh` uses the
local no-store server to avoid stale development assets. After rebuilding, reopen
`afroman.html?v=<build-web/build-id.txt value>`; a running page still needs a reload
to replace its already-loaded runtime. Compare Diagnostics IDs between previews.

## Bin landing and car headlight follow-up

Bin feedback now follows actual resolved wall/ground contacts. Slow descents
count as landings; continuing contact is latched, and side impacts require motion
toward the bin from outside, preventing walk-off feedback. The time cooldown
is removed so a new jump/landing can react immediately. Metal audio now emphasizes
inharmonic 583–4133 Hz resonances; slosh is unchanged.

Landing on the blue car toggles its front headlamp and pale beam. Its authored
collision tiles span columns 137–155; continued standing/walking on it does not
retoggle the lamp. Both features remain browser-only. Build verification passes;
interactive gameplay validation remains to be confirmed.

Title guitar now starts 200 ms before neon onset (3.8 seconds into the intro),
with the visual animation and other title cues unchanged.

## Stable bin impacts and aligned headlights

The bin uses a swept top-plane landing check and a persistent support latch.
Tile seams cannot rearm it while walking; leaving horizontally or raising the
feet more than two world pixels above the lid rearms the next landing. A focused
regression test covers initial landing, repeated seam gaps, jumping/relanding,
and stepping off without a new impact. Run with:
`clang++ -std=c++11 -IClasses proj.web/tests/surface-impact.test.cpp -o /tmp/afroman-impact-test && /tmp/afroman-impact-test`.

The headlight beam begins at the existing white pixels (native x2484..2495,
y216..227), with no new lamp square. It extends to world x5600 at the bin and
fades through 128 progressively transparent strips to zero opacity. Each light
toggle plays `Resources/web-audio/car-alarm.wav`, an original synthesized short
warbling chirp. Replace this file and rebuild to use a sourced recording.
Build and contact-state regression test pass; live gameplay validation remains
necessary for the final feel and appearance.

Guitar timing adjustment: advance reduced from 200 ms to 50 ms before neon
onset (3.95 seconds into the intro), following listening feedback.

## Solid beam, original foot support, and horn

The beam is one uniformly colored translucent polygon instead of adjacent
strips, eliminating strip seams. It retains its existing lamp origin and reaches
the bin. Car toggles now use an original synthesized steady two-tone horn beep
(`web-audio/car-horn.wav`) instead of the alarm. Replace this WAV and rebuild
to substitute another horn recording.

Bin support now uses the legacy inner foot probe (3/7 or 4/7 of sprite width,
according to facing), rather than the full collider overlapping the bin. When
that probe leaves the authored 5600..5760 top, old wall resolution is prevented
from snapping the player back onto the edge. The impact latch is unchanged.

Third spotlight cue advanced by 50 ms: its delay after neon onset is now 70 ms
instead of 120 ms. Guitar timing remains 50 ms before neon onset.

## Continuous bin collision fix

Bin wall tiles no longer participate in ground resolution; the swept continuous
top is the sole support solver. Bin tiles only block horizontal movement when
approached from outside below the lid, avoiding inclusive tile-edge wall hits
on the top and snagging on exit. Ground support is cleared before solving.
The original inner foot support range and impact latch remain. Tests cover both
edge boundaries, fast/slow downward crossings, upward motion and repeat impacts.
The horn now uses 220/277 Hz fundamentals with harmonics and lasts 380 ms.
Build and focused regression tests pass; visual gameplay confirmation remains.

## Restore confirmed bin behavior

Restored the complete collision routine, support latch, and its tests from
commit 0f3112f, the version reported to have correct contact triggering before
the narrower-edge request. This restores the earlier wider support behavior
as requested. Current solid headlights and horn playback remain. Horn pitch
returns to 440/554 Hz with a fuller harmonic blend and a shaped honk attack.

## Crowd, practice pause layering, and ladder melodies

Intro crowd gain is 0.52 (previously 0.45). Level 2 pause background/menu use
local z-orders 10001/10002, above the practice countdown at 10000.
Level 1 plays a short ascending chiptune phrase when a climb starts and a
descending phrase when a sewer descent starts; holding a key does not retrigger
the phrase throughout the traversal. Original synthesized placeholders are
`Resources/web-audio/ladder-up.wav` and `ladder-down.wav`; replace either file
and rebuild to supply your own audio. `generate_ladder_audio.py` restores both.

## Level 1 concert free play

The first red curtain starts at native map x7472 (world14944); the final curtain
section is centered at world15472. Overlapping its left boundary upstairs while
time remains secures victory. Camera pans there over 750 ms, the old HUD clock
slides left at the same rate, and the new white seconds-only clock appears in an
80×80 black square at (15472,1524). Player bounds are curtain-left to map-right.
The existing countdown continues, pauses with the game, and opens YOU WIN at
zero; Enter skips directly to that menu. Without stage arrival, zero still loses.

Stage drumming uses Level 2 keys: U cymbal, J hi-hat, F snare, R/T/Y toms, Space
bass. Background music stops at entry to leave room for free play. The bottom
label reads FREE PLAY - PRESS [ENTER] TO SKIP. The opening objective is
Get to the concert before time runs out. Level 1 movement accepts WASD as well
as arrows. Every newly triggered flush adds 30 seconds, retaining fractional
countdown progress. Holding contact does not repeatedly grant time.

Validation: browser build and concert.test.cpp pass (arrival threshold, wrong
floor, secured win, late arrival/loss, and fractional flush bonus). Visual stage
layout and complete gameplay traversal remain to be checked interactively.

## Concert layout refinement

Opening objective uses the timer's 40px font and camera-relative y280, with an
800×48 left-aligned container beginning 20px from the screen edge. Concert pan
travels at 480 world pixels/second, matching normal running (8px at 60Hz). The
stage clock center is y1536, halfway between the red panel bottom1472 and screen
top1600; its black square follows. Free-play text is 40px at y992, the center
of the brown strip960..1024. Only the drumset sprite moves left: the white kick
circle at source x140 now aligns with final camera center15472.

## Stage entrance and objective follow-up

Objective font is now 32px, shifted right clear of Pause, with a 700px container.
After five unpaused gameplay seconds it fades over 400ms. Stage entry hides the
old HUD timer immediately. Camera pan begins from the currently displayed Follow
transform rather than the next target position, preventing a curtain jump.
The player disappears at entry, waits for camera arrival plus one second, then
rises/fades into view over 500ms at stage center. Movement waits until this
entrance completes. The stage countdown keeps running throughout. YOU WIN hides
the free-play skip label, including when Enter skips early. Browser build and
concert logic tests pass; visual timing/layout still needs a gameplay check.

## Stage entry polish

YOU WIN sound now plays immediately at stage entry and does not repeat when
the final win menu opens. Player remains frozen/visible at the entry world
position during the pan; the later rise uses idle torso source-x125 as its
center reference. Movement bounds now keep the sprite inside the drumset's
actual bounding box. Objective matches the timer's exact 40px font, 32px text
container height, and camera-relative y280; the earlier 48px container caused
a baseline mismatch. Existing objective fade and horizontal placement remain.

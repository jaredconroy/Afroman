# Browser rhythm timing and exit behavior

## Clock and chart

Level 2 uses Web Audio playback seconds for both chart position and hit judgment.
It does not accumulate movement per rendered frame. Dropped frames skip drawing;
they do not slow the chart relative to music. The map's original note positions
and nominal 8.2 * 60 = 492 pixels/second pace are retained as the baseline.
This is not a claim that the 2014 chart is perfectly authored against the track.

Web Audio getOutputTimestamp is preferred to estimate the sound currently reaching
the device; baseLatency/outputLatency provide a fallback. Pause/resume retains
playback position, restart starts a new transport, and hidden tabs suspend audio.
The chart has a short lead-out past the non-looping song, so the transport remains
available until the chart finishes. The song no longer starts a second loop there.

Note centers cross the key-box centers using the same time equation used by hit
judgment. Hits are tested at keydown within +/-90 ms, independent of the last drawn
frame. Each note may score once, including when users repeatedly tap a key inside
one hit window. Note columns are cached when the level loads instead of scanning
the TMX layers every frame.

## Calibration

Open "Level 2 timing adjustment" below the canvas. The offset defaults to 0 ms,
ranges from -1000 to +1000 ms in 10 ms steps, and is saved in this browser when
storage is available. Positive means notes arrive later relative to music;
negative means earlier. This changes both visible crossings and hit times.

A consistent early/late feel can use this offset. If alignment changes progressively
from the beginning to the end, that is a chart tempo/spacing issue: calibrate the
chart's pixelsPerSecond and/or author note times against the actual track. Do not
hide that problem with one laptop-specific song delay. Listening/playtesting is
still needed to choose a final authored baseline. Audio output latency estimation
does not eliminate keyboard, display or Bluetooth latency on every device.

## Visual feedback

Each drum press pulses its letter square and flashes a 50×50 border matching the
white key frame: neon green for a scored hit, bright red for an off-time press,
and neutral white during the controls intro. A matched note briefly becomes fully
opaque, then stays opaque until it leaves the screen, with the same
colored border. Off-time presses also flash a nearby unhit note within 300 ms;
more distant notes are left alone. Borders are 4.5 px thick (previously 2.5 px)
and expire in about 410 ms; key squares pulse to 122% scale and repeated
presses replace the previous key flash. Feedback reads the existing judgment;
the +/-90 ms hit window, scoring, audio clock, and calibration are unchanged.

SELECT LEVEL in every pause/end menu opens the three level buttons directly.
The opening animation remains available on initial launch and EXIT.

## EXIT

All browser EXIT callbacks stop music/effects and replace the scene with a fresh
title. They never call Director::end or cancel the browser loop. Native exit paths
remain behind the original platform logic. The separate archived Mac checkout
and engine have not been edited.

## Checks

```sh
.tools/emsdk/node/24.19.0_64bit/bin/node proj.web/tests/audio.test.cjs
clang++ -std=c++11 -IClasses proj.web/tests/rhythm.test.cpp -o /tmp/afroman-rhythm-test
/tmp/afroman-rhythm-test
./proj.web/build.sh Debug
```

Deterministic checks cover output-clock preference/fallback, pause/resume,
restart/stop transport state, chart lead-out, offsets, chart crossing at 15-144 Hz,
judging independently of a stale frame, and one hit per note. These do not substitute
for a listening check of chart-to-track alignment.

Safari regression testing (2026-10-17): level 1 pause/EXIT returned to an interactive
level-selection menu; level 2 rendered moving notes and accepted input; pause,
resume and restart remained functional. Final musical alignment was not certified.

Safari check (2026-10-17): level 2 pause/SELECT LEVEL opened the three-button
selection screen without replaying the title animation.

The practice countdown follows chart-entry time and calibration; the visible
five-second count includes the existing scene transition lead-in. Results retain
the original three buttons and allow unscored drumming without dismissing the
score panel. The results panel no longer dims the surrounding drum kit.

# Afroman Web — title and menu milestone

This is an isolated checkout at ~/Documents/Afroman-web, branch codex/web-port.
The Mac preservation checkout and its engine are not used by this build.

## Build and run

Prerequisites: Git, Python 3, and FFmpeg on PATH (tested with FFmpeg 8.1.1).
FFmpeg is needed at build time only; browser users do not install it.

```sh
./proj.web/bootstrap.sh   # first setup; downloads SDK and pinned upstream sources
./proj.web/build.sh Debug
./proj.web/serve.sh
```

Open http://127.0.0.1:8000/afroman.html. PORT can override the local port.
Use ./proj.web/build.sh Release for an optimized compilation. This milestone is
a development game build, not the final portfolio release.

Output: build-web/afroman.html, afroman.js, afroman.wasm, and afroman.data.
All four files are needed on a static host. Assets are packaged automatically
from Resources at /Resources in Emscripten's filesystem. The packaging step
stages a separate generated tree and converts song.mp3 to 16-bit stereo PCM WAV
(44.1 kHz, 31.739 seconds). Safari rejects the original MP3, and FFmpeg reports
an invalid-backstep warning while decoding its legacy stream. The original
Resources/song.mp3 is unchanged; the audio adapter maps that game filename to
browser-audio/song.wav. web-assets.json records original/converted checksums.
The generated data package is larger until production compression is addressed.
Serve over HTTP;
do not open the HTML as a file:// URL. The Python server supplies the WASM MIME type.

## Pinned inputs

- Game source starts from Mac preservation commit 3beff67.
- Cocos2d-x 3.2: 2210d1fe87c3dd7d2ce79d01945b85a0de24862d.
- Emscripten 4.0.20; installed locally in .tools/emsdk, no shell startup edits.
- Initial SDK manager revision: 1c9a998ee5dd2e38aec0c5abbf23dffdb8f10d57.
- TIFF 4.7.1: 5fe20d0e9aba49a6a350ed533459d1505203838f.
- WebP 1.5.0: a4d7a715337ded4451fec90ff8ce79728e04126c.
- CMake 4.4.4 and Ninja 1.13.2 in a local Python virtual environment.
- PNG, JPEG, zlib and FreeType are compiled by the pinned Emscripten ports.
- Portable tinyxml2, unzip, xxhash, ConvertUTF and edtaa3func sources are retained
  under proj.web/third_party from the restored Cocos dependency distribution.

No native Mac static libraries are linked. The generated WASM runs independently
of Rosetta. The installed SDK host tools on this machine are Intel binaries;
a future native ARM SDK setup can replace those without changing the WASM target.

## Browser layer and engine patch

proj.web/platform supplies canvas/WebGL context creation, browser event callbacks,
application main-loop scheduling, virtual filesystem paths, and FreeType text.
The text adapter reads the bundled pixel font without requiring installation.
Text layout is provisional: ASCII HUD text, monochrome glyphs, basic alignment;
stroke/shadow, multiline layout and final HUD placement still need validation.
SimpleAudioEngine now has a Web Audio backend with music/effect playback,
pause/resume/rewind/stop and volume controls. The Play button resumes AudioContext
and decodes all ten bundled audio files before calling the C++ main function.
Missing/undecodable audio is an explicit startup error. Audio playback, latency,
background-tab behavior and level 2 synchronization still require verification.

The main target compiles all seven original game translation units and starts
AppDelegate/StartScreen. The original animated title and three-level selection menu
render with real JPEG/PNG resources. Enter opens the menu; mouse clicks select levels.
Ordinary and keypad Enter map to the game's existing keypad-Enter constant.
Pointer coordinates account for CSS scaling; keyboard events are handled while the
canvas is focused and held keys are released on blur. Repeated Enter cannot stack
duplicate selection menus. Arrow keys control movement/jumping; level 2 drums use
Space/F/J/R/T/Y/U. The milestone-1 scene is retained as smoke.cpp for reference.

cocos-web.patch applies only to the pristine pinned web engine. It changes platform
detection to __EMSCRIPTEN__, exposes browser platform headers, fixes obsolete nullptr
comparisons rejected by modern Clang, and uses glBufferSubData instead of mapped
buffer uploads for WebGL. Historical EMSCRIPTEN branches are left inactive using
-UEMSCRIPTEN: they refer to missing classes and an obsolete rendering design.

FULL_ES2 enables Emscripten's GLES2 client-array compatibility needed by Cocos 3.2
LayerColor and other older drawing paths. VAO acceleration uses the engine's existing
non-VAO configuration. Physics and script bindings are not built; Afroman uses its
own gameplay/collision code and no scripting. The image codecs remain compiled;
TIFF optional external compression integrations are not enabled in this prototype.

## Next milestones

Validate full level-one gameplay/collision/pause/restart, then remaining HUDs,
audio playback/unlock/pause/resume, all three levels, and production packaging.
Level 2's old Windows frame-dependent timing is not authoritative; recalibrate
against the audio clock rather than preserving existing drift. Current browser
exit buttons still follow legacy process-exit behavior and need a browser return
flow before considering the game complete.

## Milestone 1 validation (2026-10-16)

Debug WebAssembly compilation and linking succeeded with modern Clang. Safari
loaded the static HTML/JS/WASM/data bundle and reported WebGL 1 / GLES2. Visual
inspection verified the background layer and rotating green polygon, exercising
both legacy client arrays and VBO-backed drawing. This validates canvas creation
and the Cocos scheduler/action/render loop, not Afroman gameplay or audio.

## Title/menu validation (2026-10-16)

- All seven game sources compiled and linked as WebAssembly.
- Safari decoded all ten packaged audio files with a running AudioContext.
- Real title intro/loop rendered; Enter opened the three-level selection menu.
- Clicking level 1 loaded its TMX maps, sprite atlas and white pixel-font timer.
- Holding Right moved the player/camera without scrolling the page.
- Resource audit found every TMX-referenced image and every staged source asset,
  apart from the documented MP3-to-WAV alias.

These are smoke checks, not a full level playthrough. Audio decoding is verified;
actual output/latency and rhythm synchronization are not yet verified.

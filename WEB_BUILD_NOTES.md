# Afroman Web — milestone 1

This is an isolated checkout at ~/Documents/Afroman-web, branch codex/web-port.
The Mac preservation checkout and its engine are not used by this build.

## Build and run

```sh
./proj.web/bootstrap.sh   # first setup; downloads SDK and pinned upstream sources
./proj.web/build.sh Debug
./proj.web/serve.sh
```

Open http://127.0.0.1:8000/afroman.html. PORT can override the local port.
Use ./proj.web/build.sh Release for an optimized compilation. This milestone is
a development engine test, not the final portfolio release.

Output: build-web/afroman.html, afroman.js, afroman.wasm, and afroman.data.
All four files are needed on a static host. Assets are packaged automatically
from Resources at /Resources in Emscripten's filesystem. Serve over HTTP;
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
Audio is not yet wired into this smoke target. No game audio implementation has
been replaced with silent stubs.

The main target currently runs a Cocos scene with a background and rotating green
polygon. It does not compile Afroman's scenes yet. Original game sources are retained
for the next milestone. The browser platform input adapter is present but gameplay
input and scaled pointer coordinates still need testing and refinement.

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

Bring in the seven authoritative game translation units, real title assets, and
SimpleAudioEngine browser backend; validate title/menu input and level one.
Then validate fonts, audio unlock/pause/resume, all levels, and production packaging.
Level 2's old Windows frame-dependent timing is not authoritative; recalibrate
against the audio clock rather than preserving existing drift.

## Milestone 1 validation (2026-10-16)

Debug WebAssembly compilation and linking succeeded with modern Clang. Safari
loaded the static HTML/JS/WASM/data bundle and reported WebGL 1 / GLES2. Visual
inspection verified the background layer and rotating green polygon, exercising
both legacy client arrays and VBO-backed drawing. This validates canvas creation
and the Cocos scheduler/action/render loop, not Afroman gameplay or audio.

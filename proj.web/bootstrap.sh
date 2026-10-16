#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
command -v ffmpeg >/dev/null || { echo "FFmpeg is required to package the browser music asset." >&2; exit 1; }
mkdir -p .tools vendor
clone_pinned() {
 local url="$1" tag="$2" revision="$3" destination="$4"
 if [ ! -d "$destination/.git" ]; then
  git clone --depth 1 --branch "$tag" "$url" "$destination"
 fi
 test "$(git -C "$destination" rev-parse HEAD)" = "$revision" || { echo "Unexpected revision in $destination" >&2; exit 1; }
}
clone_pinned https://github.com/cocos2d/cocos2d-x.git cocos2d-x-3.2 2210d1fe87c3dd7d2ce79d01945b85a0de24862d vendor/cocos2d
clone_pinned https://github.com/libsdl-org/libtiff.git v4.7.1 5fe20d0e9aba49a6a350ed533459d1505203838f vendor/libtiff
clone_pinned https://github.com/webmproject/libwebp.git v1.5.0 a4d7a715337ded4451fec90ff8ce79728e04126c vendor/libwebp
if git -C vendor/cocos2d apply --reverse --check ../../proj.web/cocos-web.patch 2>/dev/null; then
 echo "Cocos web patch already applied"
else
 git -C vendor/cocos2d apply --check ../../proj.web/cocos-web.patch
 git -C vendor/cocos2d apply ../../proj.web/cocos-web.patch
fi
if [ ! -d .tools/emsdk/.git ]; then git clone https://github.com/emscripten-core/emsdk.git .tools/emsdk; fi
.tools/emsdk/emsdk install 4.0.20
.tools/emsdk/emsdk activate 4.0.20
if [ ! -f .tools/python/bin/python3 ]; then python3 -m venv .tools/python; fi
.tools/python/bin/pip install cmake==4.4.4 ninja==1.13.2

#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
export PATH="$PWD/.tools/python/bin:$PATH"
source .tools/emsdk/emsdk_env.sh >/dev/null 2>&1
configuration="${1:-Debug}"
case "$configuration" in Debug|Release) ;; *) echo "Usage: $0 [Debug|Release]" >&2; exit 2;; esac
emcmake cmake -S proj.web -B "build-web" -G Ninja -DCMAKE_BUILD_TYPE="$configuration"
cmake --build "build-web" --parallel 6

python3 proj.web/stamp_build.py build-web

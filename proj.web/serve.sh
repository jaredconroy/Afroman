#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
exec python3 proj.web/serve.py --port "${PORT:-8000}" --directory build-web

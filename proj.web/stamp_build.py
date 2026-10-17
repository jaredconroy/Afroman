#!/usr/bin/env python3
"""Fingerprint emitted artifacts, including builds where Ninja did not relink."""
import hashlib,re,sys
from pathlib import Path
root=Path(sys.argv[1]);digest=hashlib.sha256()
for name in ['afroman.js','afroman.wasm','afroman.data']:
    digest.update((root/name).read_bytes())
version=digest.hexdigest()[:12]
p=root/'afroman.html';s=p.read_text()
s=re.sub(r"const buildId = '[^']*';",f"const buildId = '{version}';",s)
s=re.sub(r'(<span id="build-id">)[^<]*(</span>)',lambda m:m[1]+version+m[2],s)
s=re.sub(r'(src=["\'])afroman\.js(?:\?[^"\']*)?(["\'])',lambda m:m[1]+'afroman.js?v='+version+m[2],s)
p.write_text(s)
(root/'build-id.txt').write_text(version+'\n')
print('Browser build:',version)

#!/usr/bin/env python3
"""Local static server: never reuse stale game code/assets during development."""
import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler,ThreadingHTTPServer
class Handler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cache-Control','no-store, max-age=0')
        super().end_headers()
    def do_GET(self):
        if 'If-Modified-Since' in self.headers: del self.headers['If-Modified-Since']
        super().do_GET()
parser=argparse.ArgumentParser()
parser.add_argument('--port',type=int,default=8000)
parser.add_argument('--directory',default='build-web')
args=parser.parse_args()
with ThreadingHTTPServer(('127.0.0.1',args.port),partial(Handler,directory=args.directory)) as server:
    print(f'Afroman: http://127.0.0.1:{args.port}/afroman.html',flush=True)
    server.serve_forever()

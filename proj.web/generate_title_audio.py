#!/usr/bin/env python3
"""Restore only the three original synthesized title WAV placeholders (overwrites)."""
import math
from pathlib import Path
import random
import struct
import wave
RATE=22050
out=Path(__file__).resolve().parent.parent/'Resources'/'web-audio'
out.mkdir(exist_ok=True)
rng=random.Random(2014)
def save(name, data):
    with wave.open(str(out/(name+'.wav')),'wb') as f:
        f.setparams((1,2,RATE,0,'NONE','not compressed'))
        f.writeframes(b''.join(struct.pack('<h',round(max(-.95,min(.95,x))*32767)) for x in data))
# Relay click, ballast thunk, and a tiny electrical snap.
spot=[]
for i in range(round(.3*RATE)):
    t=i/RATE
    attack=min(1,t/.001)
    spot.append(attack*(.42*(rng.random()*2-1)*math.exp(-160*t)+
        .24*math.sin(2*math.pi*95*t)*math.exp(-24*t)+
        .08*math.sin(2*math.pi*1700*t)*math.exp(-70*t)))
save('spotlight',spot)
# Glass-like inharmonic partials with a short percussive attack.
plink=[]
for i in range(round(.55*RATE)):
    t=i/RATE
    plink.append(min(1,t/.002)*sum(v*math.sin(2*math.pi*f*t)*math.exp(-d*t)
        for f,v,d in [(1760,.19,13),(2447,.13,17),(4191,.07,24)]))
save('neon-plink',plink)
# Half a second contains whole cycles at 60 Hz and each harmonic, so it loops cleanly.
save('neon-buzz',[.075*math.sin(2*math.pi*60*i/RATE)+
    .055*math.sin(2*math.pi*120*i/RATE)+.025*math.sin(2*math.pi*180*i/RATE)+
    .012*math.sin(2*math.pi*600*i/RATE) for i in range(RATE//2)])
print('Generated spotlight.wav, neon-plink.wav, neon-buzz.wav')

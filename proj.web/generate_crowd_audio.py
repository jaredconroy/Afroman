#!/usr/bin/env python3
"""Original synthesized distant crowd-like ambience placeholder, no recordings."""
import math
from pathlib import Path
import random
import struct
import wave
rate=22050
length=6
rng=random.Random(2014)
data=[]
low=0
# Soft diffuse noise plus overlapping vowel-like, gently rising cheering tones.
voices=[(rng.uniform(110,300),rng.random()*2*math.pi,rng.uniform(.18,.65)) for _ in range(18)]
for i in range(rate*length+rate//4):
    t=i/rate
    low += .12*((rng.random()*2-1)-low)
    cheer=0
    for f,phase,speed in voices:
        envelope=max(0,math.sin(2*math.pi*speed*t+phase))**3
        pitch=f*(1+.1*math.sin(2*math.pi*speed*t+phase))
        cheer+=envelope*(math.sin(2*math.pi*pitch*t+phase)+.3*math.sin(2*math.pi*2*pitch*t+phase))
    data.append(.16*low+.006*cheer)
# Blend the loop seam over 250 ms while retaining the full six-second duration.
blend=rate//4
for i in range(blend):
    amount=i/(blend-1)
    data[i]=data[-blend+i]*(1-amount)+data[i]*amount
data=data[:-blend]
path=Path(__file__).resolve().parent.parent/'Resources'/'web-audio'/'crowd.wav'
with wave.open(str(path),'wb') as f:
    f.setparams((1,2,rate,0,'NONE','not compressed'))
    f.writeframes(b''.join(struct.pack('<h',round(max(-.95,min(.95,x))*32767)) for x in data))
print('Generated',path)

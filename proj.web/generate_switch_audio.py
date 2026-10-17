#!/usr/bin/env python3
"""Restore only the original synthesized bathroom switch click placeholder."""
from pathlib import Path
import math,random,struct,wave
rate=22050
rng=random.Random(2014)
data=[]
for i in range(round(.13*rate)):
    t=i/rate
    click=0
    for start,amp in [(0,.40),(.014,.24)]:
        u=t-start
        if u>=0:
            click+=amp*(.7*(rng.random()*2-1)+.3*math.sin(2*math.pi*1500*u))*math.exp(-180*u)*min(1,u/.0005)
    data.append(click)
p=Path(__file__).resolve().parent.parent/'Resources'/'web-audio'/'lightswitch.wav'
with wave.open(str(p),'wb') as f:
    f.setparams((1,2,rate,0,'NONE','not compressed'))
    f.writeframes(b''.join(struct.pack('<h',round(x*32767)) for x in data))
print('Generated',p)

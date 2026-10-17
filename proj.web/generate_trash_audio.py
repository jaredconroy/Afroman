#!/usr/bin/env python3
"""Restore only original synthesized trash metal/slosh placeholders."""
import math,random,struct,wave
from pathlib import Path
rate=22050
rng=random.Random(2014)
out=Path(__file__).resolve().parent.parent/'Resources'/'web-audio'
def save(name,samples):
 with wave.open(str(out/(name+'.wav')),'wb') as f:
  f.setparams((1,2,rate,0,'NONE','not compressed'))
  f.writeframes(b''.join(struct.pack('<h',round(max(-.95,min(.95,x))*32767)) for x in samples))
metal=[];slosh=[];low=0
for i in range(round(.65*rate)):
 t=i/rate
 metal.append(min(1,t/.002)*(sum(v*math.sin(2*math.pi*f*t)*math.exp(-d*t)
  for f,v,d in [(172,.18,9),(389,.15,12),(731,.10,14),(1261,.06,19)])+
  .12*(rng.random()*2-1)*math.exp(-70*t)))
 low+=.16*((rng.random()*2-1)-low)
 slosh.append(min(1,t/.012)*max(0,1-t/.65)*(.30*low+.065*math.sin(2*math.pi*(85*t+15*t*t)))*
  (.6+.4*math.sin(2*math.pi*8*t)))
save('trash-metal',metal);save('trash-slosh',slosh)
print('Generated trash-metal.wav and trash-slosh.wav')

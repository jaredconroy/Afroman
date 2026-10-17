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
  for f,v,d in [(583,.09,12),(1093,.15,9),(1789,.13,12),(2711,.09,16),(4133,.05,22)])+
  .12*(rng.random()*2-1)*math.exp(-70*t)))
 low+=.16*((rng.random()*2-1)-low)
 slosh.append(min(1,t/.012)*max(0,1-t/.65)*(.30*low+.065*math.sin(2*math.pi*(85*t+15*t*t)))*
  (.6+.4*math.sin(2*math.pi*8*t)))
save('trash-metal',metal) # Preserve the existing slosh recording.
print('Generated trash-metal.wav; existing trash-slosh.wav preserved')

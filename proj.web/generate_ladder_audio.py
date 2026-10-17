#!/usr/bin/env python3
"""Restore original synthesized chiptune ladder melodies."""
import math,struct,wave
from pathlib import Path
rate=44100
out=Path(__file__).resolve().parent.parent/'Resources/web-audio'
for name,notes in [('ladder-up',[60,64,67,72,76,79]),('ladder-down',[79,76,72,67,64,60])]:
 samples=[]
 for note in notes:
  freq=440*2**((note-69)/12)
  for i in range(int(.105*rate)):
   t=i/rate;env=min(1,t/.005)*max(0,1-t/.105)**.7
   # Band-limited pulse-like tone, avoiding harsh full-band square waves.
   v=.28*env*sum(math.sin(2*math.pi*freq*k*t)/k for k in [1,3,5])
   samples.append(round(v*32767))
 with wave.open(str(out/(name+'.wav')),'wb') as f:
  f.setparams((1,2,rate,0,'NONE','not compressed'))
  f.writeframes(struct.pack('<%dh'%len(samples),*samples))

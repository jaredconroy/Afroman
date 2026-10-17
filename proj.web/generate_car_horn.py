#!/usr/bin/env python3
"""Restore the original synthesized horn placeholder; no sourced recordings."""
import math,struct,wave
from pathlib import Path
rate=44100
samples=[]
phases=[0.,0.]
for i in range(int(.38*rate)):
 t=i/rate
 env=min(1,t/.014)*min(1,(.38-t)/.06)
 # Stable automotive two-tone pitch, with reed-like harmonics and a rounded attack.
 bend=1-.025*math.exp(-t*45)
 v=0
 for j,(freq,weight) in enumerate([(440,1),(554.37,.8)]):
  phases[j]+=2*math.pi*freq*bend/rate
  v+=weight*sum(amp*math.sin(k*phases[j]) for k,amp in [(1,1),(2,.48),(3,.3),(4,.16),(5,.08)])
 v=.27*env*math.tanh(v*1.15)
 samples.append(round(v*32767))
p=Path(__file__).resolve().parent.parent/'Resources/web-audio/car-horn.wav'
with wave.open(str(p),'wb') as f:
 f.setparams((1,2,rate,0,'NONE','not compressed'))
 f.writeframes(struct.pack('<%dh'%len(samples),*samples))

#!/usr/bin/env python3
"""Original synthesized placeholders; run manually to restore defaults (overwrites files)."""
import math
from pathlib import Path
import random
import struct
import wave

RATE = 22050
OUTPUT = Path(__file__).resolve().parent.parent / 'Resources' / 'web-audio'
OUTPUT.mkdir(exist_ok=True)
rng = random.Random(2014)

def save(name, samples):
    with wave.open(str(OUTPUT / (name + '.wav')), 'wb') as out:
        out.setparams((1, 2, RATE, 0, 'NONE', 'not compressed'))
        out.writeframes(b''.join(struct.pack('<h', round(max(-0.95, min(0.95, x))*32767)) for x in samples))

def freq(note):
    return 440 * 2**((note-69)/12)

def tone(buffer, start, duration, note, volume, pulse=True):
    start = round(start*RATE)
    for i in range(round(duration*RATE)):
        if start+i >= len(buffer): break
        t = i/RATE
        env = min(1, t/.008) * min(1, max(0, (duration-t)/.04))
        phase = (t*freq(note)) % 1
        shape = (1 if phase < .25 else -1/3) if pulse else 1-4*abs(phase-.5)
        buffer[start+i] += volume*env*shape

def music(name, bpm, roots, melody):
    beat = 60/bpm
    data = [0.] * round(8*4*beat*RATE)
    for bar in range(8):
        root = roots[bar % 4]
        for step in range(8):
            t = (bar*4+step*.5)*beat
            tone(data,t,beat*.42,root+(12 if step%2 else 0),.10,False)
            tone(data,t,beat*.35,melody[(bar*8+step)%len(melody)],.075)
        for b in range(4):
            t=(bar*4+b)*beat
            for i in range(round(.09*RATE)):
                j=round(t*RATE)+i
                if j>=len(data):break
                u=i/RATE
                kick=math.sin(2*math.pi*(90*u-230*u*u))*math.exp(-45*u)*.10
                noise=(rng.random()*2-1)*math.exp(-65*u)*(.045 if b%2 else .018)
                data[j]+=kick+noise
    save(name,data)

music('level1',120,[48,45,53,43],[72,76,79,76,74,76,79,83,72,76,81,76,74,72,69,67])
music('level3',100,[45,41,48,43],[81,76,79,76,77,72,76,72,79,74,77,74,76,71,74,71])
# Rising square chirp: jump/bounce.
save('jump',[(.22 if math.sin(2*math.pi*(220*(i/RATE)+950*(i/RATE)**2))>0 else -.22)*
             min(1,i/(RATE*.008))*max(0,1-i/(RATE*.19)) for i in range(round(RATE*.19))])
save('land',[(.28*math.sin(2*math.pi*65*i/RATE)+.08*(rng.random()*2-1))*
             math.exp(-30*i/RATE)*min(1,i/(RATE*.004)) for i in range(round(RATE*.18))])
# Sweeping filtered noise and descending bubbles: intentionally cartoonish flush.
flush=[]; low=0.
for i in range(round(1.8*RATE)):
    t=i/RATE
    low += (.10+.23*(1-t/1.8))*((rng.random()*2-1)-low)
    env=min(1,t/.08)*min(1,(1.8-t)/.4)
    bubble=math.sin(2*math.pi*(150*t-30*t*t))*(.5+.5*math.sin(2*math.pi*9*t))
    flush.append((low*.45+bubble*.09)*env)
save('flush',flush)
for name,notes in [('win',[72,76,79,84]),('lose',[60,55,48])]:
    d=[0.]*round(.7*RATE)
    for n,note in enumerate(notes):tone(d,n*.12,.20,note,.18)
    save(name,d)
print('Generated seven original PCM WAV placeholders in',OUTPUT)

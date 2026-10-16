// Exercise pause/resume offsets and music/effect isolation without browser timing.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
vm.runInThisContext(fs.readFileSync(path.join(__dirname, '../audio.js'), 'utf8'));
const starts = [];
const node = () => ({connect() {}, disconnect() {}, gain: {value: 1}, pan: {value: 0}});
const audio = globalThis.AfromanAudio;
audio.context = {
  currentTime: 0, state: 'running',
  createGain: node, createStereoPanner: node,
  createBufferSource() {
    return {...node(), playbackRate: {value: 1}, stop() {},
      start(when, offset) { starts.push({offset, rate: this.playbackRate.value}); }};
  }
};
audio.musicGain = node(); audio.effectsGain = node();
audio.decoded.set('/effect.wav', {duration: 10});
audio.decoded.set('/music.wav', {duration: 20});
const effect = audio.play('/effect.wav', false, 2, 0, 1, false);
audio.context.currentTime = 3; audio.pause(effect);
audio.context.currentTime = 8; audio.resume(effect);
assert.deepEqual(starts.at(-1), {offset: 6, rate: 2}, 'pause excludes elapsed paused time and honors pitch');
audio.play('/music.wav', true, 1, 0, 1, true);
audio.effects('pause');
assert.equal(audio.playing(), true, 'pausing effects leaves music playing');
audio.pause(0); assert.equal(audio.playing(), false);
audio.resume(0); assert.equal(audio.playing(), true);
audio.context.currentTime = 12; audio.rewind();
assert.equal(starts.at(-1).offset, 0, 'rewind restarts at the beginning');
audio.effects('stop'); assert.equal(audio.playing(), true, 'stopping effects leaves music playing');
audio.volume(true, 9); assert.equal(audio.musicGain.gain.value, 1);
audio.volume(false, -1); assert.equal(audio.effectsGain.gain.value, 0);
assert.throws(() => audio.buffer('/missing.wav'), /not decoded/);
audio.end(); assert.equal(audio.playing(), false);
console.log('PASS: audio pause/resume, rewind, effect isolation, volume bounds, missing-asset failure');

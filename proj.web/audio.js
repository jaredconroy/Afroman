// Decode the real bundled audio before entering C++, so SimpleAudioEngine's
// synchronous interface never races an asynchronous browser decode.
globalThis.AfromanAudio = {
  context: null, decoded: new Map(), active: new Map(), voices: new Map(), nextId: 1,
  musicVolume: 1, effectsVolume: 1, syncOffsetMs: 0,
  async prepare() {
    if (!this.context) {
      this.context = new AudioContext({latencyHint: 'interactive'});
      this.musicGain = this.context.createGain();
      this.effectsGain = this.context.createGain();
      this.musicGain.connect(this.context.destination);
      this.effectsGain.connect(this.context.destination);
    }
    await this.context.resume();
    if (this.context.state !== 'running') throw new Error('Audio could not start. Please click Play again.');
    const paths = [];
    const walk = dir => FS.readdir(dir).filter(n => n !== '.' && n !== '..').forEach(n => {
      const path = dir + '/' + n;
      if (FS.isDir(FS.stat(path).mode)) walk(path);
      else if (/\.(wav|mp3|ogg)$/i.test(n)) paths.push(path);
    });
    walk('/Resources');
    await Promise.all(paths.map(async path => {
      if (!this.decoded.has(path)) {
        const bytes = FS.readFile(path);
        try {
          this.decoded.set(path, await this.context.decodeAudioData(bytes.slice().buffer));
        } catch (error) { throw new Error(path + ": " + error.message); }
      }
    }));
    Module.print('AFROMAN_AUDIO: decoded ' + this.decoded.size + ' bundled files; context ' + this.context.state);
  },
  buffer(path) {
    if (!this.active.has(path)) {
      const buffer = this.decoded.get(path);
      if (!buffer) throw new Error('Audio asset was not decoded: ' + path);
      this.active.set(path, buffer);
    }
    return this.active.get(path);
  },
  start(voice) {
    const context = this.context, source = context.createBufferSource();
    source.buffer = voice.buffer; source.loop = voice.loop;
    source.playbackRate.value = voice.pitch;
    const gain = context.createGain(); gain.gain.value = voice.gain;
    const pan = context.createStereoPanner(); pan.pan.value = voice.pan;
    source.connect(gain); gain.connect(pan);
    pan.connect(voice.id === 0 ? this.musicGain : this.effectsGain);
    voice.source = source; voice.gainNode = gain; voice.panNode = pan;
    voice.started = context.currentTime; voice.paused = false; voice.ended = false;
    source.onended = () => {
      if (voice.source !== source) return;
      if (voice.id === 0 && !voice.loop) {
        // Keep the transport clock through the chart's short trailing section.
        this.detach(voice); voice.ended = true;
      } else this.stop(voice.id);
    };
    source.start(0, voice.offset);
  },
  play(path, loop, pitch, pan, gain, music) {
    if (music) this.stop(0);
    const id = music ? 0 : this.nextId++;
    const voice = {id, buffer: this.buffer(path), loop: !!loop, pitch: Math.max(0.01, pitch),
      pan: Math.max(-1, Math.min(1, pan)), gain: Math.max(0, Math.min(1, gain)), offset: 0};
    this.voices.set(id, voice); this.start(voice); return id;
  },
  detach(voice) {
    if (voice.source) {
      voice.source.onended = null; voice.source.stop(); voice.source.disconnect();
      voice.gainNode.disconnect(); voice.panNode.disconnect(); voice.source = null;
    }
  },
  pause(id) {
    const v = this.voices.get(id); if (!v || v.paused) return;
    v.offset += (this.context.currentTime - v.started) * v.pitch;
    if (v.loop) v.offset %= v.buffer.duration;
    this.detach(v); v.paused = true;
  },
  resume(id) {
    const v = this.voices.get(id); if (!v || !v.paused) return;
    if (v.offset >= v.buffer.duration) {
      if (id === 0 && !v.loop) { v.started = this.context.currentTime; v.paused = false; v.ended = true; }
      else this.stop(id);
      return;
    }
    this.start(v);
  },
  stop(id) { const v = this.voices.get(id); if (v) { this.detach(v); this.voices.delete(id); } },
  effects(action) { for (const id of [...this.voices.keys()]) if (id !== 0) this[action](id); },
  rewind() { const v = this.voices.get(0); if (v) { this.detach(v); v.offset = 0; this.start(v); } },
  // Prefer the audio device's presentation clock over the render-ahead clock.
  outputTime() {
    const c = this.context;
    if (c.getOutputTimestamp) {
      const stamp = c.getOutputTimestamp(), now = performance.now();
      if (stamp.contextTime > 0 && stamp.performanceTime > 0 && now-stamp.performanceTime < 250)
        return Math.max(0, Math.min(c.currentTime, stamp.contextTime+(now-stamp.performanceTime)/1000));
    }
    return Math.max(0, c.currentTime-(c.baseLatency||0)-(c.outputLatency||0));
  },
  musicPosition() {
    const v = this.voices.get(0);
    if (!v) return 0;
    if (v.paused) return v.offset;
    let position = v.offset + Math.max(0, this.outputTime()-v.started)*v.pitch;
    return v.loop ? position % v.buffer.duration : position;
  },
  playing() { const v = this.voices.get(0); return !!v && !v.paused && !v.ended && this.context.state === 'running'; },
  volume(music, value) {
    value = Math.max(0, Math.min(1, value));
    this[music ? 'musicVolume' : 'effectsVolume'] = value;
    this[music ? 'musicGain' : 'effectsGain'].gain.value = value;
  },
  end() { for (const id of [...this.voices.keys()]) this.stop(id); this.active.clear(); }
};

#include "SimpleAudioEngine.h"
#include "platform/CCFileUtils.h"
#include <emscripten.h>
#include <string>
namespace {
std::string pathFor(const char* path) { return cocos2d::FileUtils::getInstance()->fullPathForFilename(std::string(path)=="song.mp3" ? "browser-audio/song.wav" : path); }
EM_JS(void, preload, (const char* path), { globalThis.AfromanAudio.buffer(UTF8ToString(path)); });
EM_JS(unsigned, play, (const char* path, int loop, float pitch, float pan, float gain, int music), {
 return globalThis.AfromanAudio.play(UTF8ToString(path),loop,pitch,pan,gain,music);
});
EM_JS(void, action, (int op, unsigned id), {
 const a=globalThis.AfromanAudio;
 switch(op) {
 case 0:a.pause(id);break; case 1:a.resume(id);break; case 2:a.stop(id);break;
 case 3:a.effects('pause');break; case 4:a.effects('resume');break; case 5:a.effects('stop');break;
 case 6:a.rewind();break; case 7:a.end();break;
 }
});
EM_JS(void, unload, (const char* path), { globalThis.AfromanAudio.active.delete(UTF8ToString(path)); });
EM_JS(int, musicPlaying, (), { return globalThis.AfromanAudio.playing(); });
EM_JS(float, getVolume, (int music), { return globalThis.AfromanAudio[music?'musicVolume':'effectsVolume']; });
EM_JS(void, setVolume, (int music,float value), { globalThis.AfromanAudio.volume(music,value); });
CocosDenshion::SimpleAudioEngine* instance=nullptr;
}
namespace CocosDenshion {
SimpleAudioEngine::SimpleAudioEngine() {}
SimpleAudioEngine::~SimpleAudioEngine() {}
SimpleAudioEngine* SimpleAudioEngine::getInstance() { if(!instance) instance=new SimpleAudioEngine(); return instance; }
void SimpleAudioEngine::end() { action(7,0); delete instance; instance=nullptr; }
void SimpleAudioEngine::preloadBackgroundMusic(const char* path) { preload(pathFor(path).c_str()); }
void SimpleAudioEngine::playBackgroundMusic(const char* path,bool loop) { play(pathFor(path).c_str(),loop,1,0,1,1); }
void SimpleAudioEngine::stopBackgroundMusic(bool release) { action(2,0); }
void SimpleAudioEngine::pauseBackgroundMusic() { action(0,0); }
void SimpleAudioEngine::resumeBackgroundMusic() { action(1,0); }
void SimpleAudioEngine::rewindBackgroundMusic() { action(6,0); }
bool SimpleAudioEngine::willPlayBackgroundMusic() { return true; }
bool SimpleAudioEngine::isBackgroundMusicPlaying() { return musicPlaying(); }
float SimpleAudioEngine::getBackgroundMusicVolume() { return getVolume(1); }
void SimpleAudioEngine::setBackgroundMusicVolume(float value) { setVolume(1,value); }
float SimpleAudioEngine::getEffectsVolume() { return getVolume(0); }
void SimpleAudioEngine::setEffectsVolume(float value) { setVolume(0,value); }
unsigned SimpleAudioEngine::playEffect(const char* path,bool loop,float pitch,float pan,float gain) { return play(pathFor(path).c_str(),loop,pitch,pan,gain,0); }
void SimpleAudioEngine::pauseEffect(unsigned id) { action(0,id); }
void SimpleAudioEngine::resumeEffect(unsigned id) { action(1,id); }
void SimpleAudioEngine::stopEffect(unsigned id) { action(2,id); }
void SimpleAudioEngine::pauseAllEffects() { action(3,0); }
void SimpleAudioEngine::resumeAllEffects() { action(4,0); }
void SimpleAudioEngine::stopAllEffects() { action(5,0); }
void SimpleAudioEngine::preloadEffect(const char* path) { preload(pathFor(path).c_str()); }
void SimpleAudioEngine::unloadEffect(const char* path) { unload(pathFor(path).c_str()); }
}

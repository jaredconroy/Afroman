#include "WebRhythm.h"
#include <emscripten.h>
namespace {
EM_JS(double, webMusicTime, (), { return globalThis.AfromanAudio.musicPosition(); });
EM_JS(double, webCalibrationOffset, (), { return globalThis.AfromanAudio.syncOffsetMs / 1000; });
}
namespace afroman {
double musicTime() { return webMusicTime(); }
double calibrationOffset() { return webCalibrationOffset(); }
}

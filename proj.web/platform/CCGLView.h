#pragma once
#include "platform/CCGLViewProtocol.h"
#include "base/CCRef.h"
#include <emscripten/html5.h>
namespace cocos2d {
class GLView : public GLViewProtocol, public Ref {
public:
 static GLView* create(const std::string& name);
 bool isOpenGLReady() override { return context_ > 0; }
 void end() override;
 void swapBuffers() override {}
 void setIMEKeyboardState(bool) override {}
 void setFrameSize(float w, float h) override;
private:
 EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_ = 0;
};
}

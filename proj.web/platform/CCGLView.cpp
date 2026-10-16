#include "CCGLView.h"
#include "base/CCDirector.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventKeyboard.h"
#include <cstring>
namespace cocos2d {
static EM_BOOL keyboard(int type, const EmscriptenKeyboardEvent* e, void*) {
 using K = EventKeyboard::KeyCode;
 K key = K::KEY_NONE;
 if (!strcmp(e->code,"ArrowLeft")) key=K::KEY_LEFT_ARROW;
 else if (!strcmp(e->code,"ArrowRight")) key=K::KEY_RIGHT_ARROW;
 else if (!strcmp(e->code,"ArrowUp")) key=K::KEY_UP_ARROW;
 else if (!strcmp(e->code,"ArrowDown")) key=K::KEY_DOWN_ARROW;
 else if (!strcmp(e->code,"Space")) key=K::KEY_SPACE;
 else if (!strcmp(e->code,"Enter") || !strcmp(e->code,"NumpadEnter")) key=K::KEY_KP_ENTER;
 else if (!strcmp(e->code,"Escape")) key=K::KEY_ESCAPE;
 else if (e->code[0]=='K' && e->code[1]=='e' && e->code[2]=='y') key=static_cast<K>(static_cast<int>(K::KEY_A)+e->code[3]-'A');
 if(key==K::KEY_NONE) return EM_FALSE;
 if(!e->repeat) { EventKeyboard event(key,type==EMSCRIPTEN_EVENT_KEYDOWN); Director::getInstance()->getEventDispatcher()->dispatchEvent(&event); }
 return EM_TRUE;
}
static EM_BOOL mouse(int type, const EmscriptenMouseEvent* e, void* user) {
 auto view=static_cast<GLView*>(user); intptr_t id=0; float x=e->targetX,y=e->targetY;
 if(type==EMSCRIPTEN_EVENT_MOUSEDOWN) view->handleTouchesBegin(1,&id,&x,&y);
 else if(type==EMSCRIPTEN_EVENT_MOUSEUP) view->handleTouchesEnd(1,&id,&x,&y);
 else if(e->buttons&1) view->handleTouchesMove(1,&id,&x,&y);
 return EM_TRUE;
}
GLView* GLView::create(const std::string& name) {
 auto view=new GLView(); view->setViewName(name);
 EmscriptenWebGLContextAttributes a; emscripten_webgl_init_context_attributes(&a);
 a.alpha=false; a.depth=true; a.stencil=true; a.antialias=false; a.majorVersion=1;
 view->context_=emscripten_webgl_create_context("#canvas",&a);
 if(view->context_<=0) { delete view; return nullptr; }
 emscripten_webgl_make_context_current(view->context_);
 view->setFrameSize(1050,640);
 emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,keyboard);
 emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,keyboard);
 emscripten_set_mousedown_callback("#canvas",view,true,mouse);
 emscripten_set_mouseup_callback("#canvas",view,true,mouse);
 emscripten_set_mousemove_callback("#canvas",view,true,mouse);
 view->autorelease(); return view;
}
void GLView::setFrameSize(float w,float h) { emscripten_set_canvas_element_size("#canvas",w,h); GLViewProtocol::setFrameSize(w,h); }
void GLView::end() { emscripten_cancel_main_loop(); }
}

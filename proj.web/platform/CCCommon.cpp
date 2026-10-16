#include "platform/CCCommon.h"
#include "base/CCConsole.h"
namespace cocos2d {
void MessageBox(const char* msg,const char* title) { log("%s: %s",title,msg); }
void LuaLog(const char* text) { log("%s",text); }
}

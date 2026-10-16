#include "CCApplication.h"
#include "base/CCDirector.h"
#include "platform/CCFileUtils.h"
#include <emscripten.h>
#include "base/CCConsole.h"
namespace cocos2d {
Application* Application::sm_pSharedApplication=nullptr;
Application::Application():_animationInterval(16667) { sm_pSharedApplication=this; }
Application::~Application() { sm_pSharedApplication=nullptr; }
Application* Application::getInstance() { return sm_pSharedApplication; }
Application* Application::sharedApplication() { return getInstance(); }
int Application::run() {
 if(!applicationDidFinishLaunching()) return 1;
 emscripten_set_main_loop([] { Director::getInstance()->mainLoop(); static int frames=0; if(++frames==60) log("AFROMAN_WEB: 60 frames rendered"); },60,true);
 return 0;
}
void Application::setAnimationInterval(double interval) { _animationInterval=interval*1000000; }
LanguageType Application::getCurrentLanguage() { return LanguageType::ENGLISH; }
const char* Application::getCurrentLanguageCode() { return "en"; }
Application::Platform Application::getTargetPlatform() { return Platform::OS_EMSCRIPTEN; }
void Application::setResourceRootPath(const std::string& path) { _resourceRootPath=path; FileUtils::getInstance()->addSearchPath(path,true); }
const std::string& Application::getResourceRootPath() { return _resourceRootPath; }
}

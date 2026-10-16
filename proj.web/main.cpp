#include "AppDelegate.h"
#include "base/CCConsole.h"
int main() {
    static AppDelegate app;
    cocos2d::log("AFROMAN_WEB: starting original game");
    return cocos2d::Application::getInstance()->run();
}

#include "AppDelegate.h"
#include "StartScreen.h"
#include "platform/desktop/CCGLView.h"

USING_NS_CC;

AppDelegate::AppDelegate() {
}

AppDelegate::~AppDelegate()
{
}

bool AppDelegate::applicationDidFinishLaunching() {

	auto director = Director::getInstance();
	auto glview = director->getOpenGLView();
	if (!glview) {
        glview = GLView::create("Afroman");
		glview->setFrameSize(1050, 640);
		director->setOpenGLView(glview);
	}

	director->setDisplayStats(false);

	director->setAnimationInterval(1.0 / 60);

	auto scene = StartScreen::createScene();

	director->runWithScene(TransitionFade::create(1.0f, scene));

	return true;
}

void AppDelegate::applicationDidEnterBackground() {
	Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
	Director::getInstance()->startAnimation();
}

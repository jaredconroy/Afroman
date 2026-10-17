#include "StartScreen.h"
#include "globals.h"
#include "SimpleAudioEngine.h"
#include "GameScene.h"
#include "GameScene2.h"
#include "GameScene3.h"
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
EM_JS(void, afroman_title_gain, (unsigned id, float value), {
    globalThis.AfromanAudio.effectGain(id,value);
});
#endif


Scene* StartScreen::createScene(bool showLevelSelection)
{
	auto scene = Scene::create();

	auto layer = StartScreen::create();

	scene->addChild(layer);
    if (showLevelSelection) {
        layer->unscheduleAllSelectors();
        layer->sprite->stopAllActions();
        layer->sprite2->stopAllActions();
        layer->sprite->removeFromParent();
        layer->selectMenu();
        layer->selectMenubg->stopAllActions();
        layer->selectMenubg->setOpacity(255);
    }

	return scene;
}

bool StartScreen::init()
{
	if (!Layer::init())
	{
		return false;
	}

	cocos2d::Animation *anim = Animation::create();

	char zname[100] = { 0 };
	
	for (int i = 0; i < 4; i++){
		sprintf(zname, "start1/start%i.jpg", i);
		anim->addSpriteFrameWithFileName(zname);
	}

	sprite = Sprite::create("start1/start0.jpg");

	this->addChild(sprite, 30, "first_anim");
	anim->setDelayPerUnit(1.0f);
	auto action = Animate::create(anim);
#if defined(__EMSCRIPTEN__)
    Vector<FiniteTimeAction*> intro;
    for (int frame=0; frame<4; ++frame) {
        intro.pushBack(CallFunc::create([this,frame]() {
            sprite->setTexture("start1/start" + to_string(frame) + ".jpg");
            if (titleAudioEnabled && frame >= 2)
                titleEffects.push_back(CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(
                    "web-audio/spotlight.wav",false,1.0f,
                    frame==2 ? -0.35f : 0.35f,0.8f));
        }));
        if (frame == 3) {
            intro.pushBack(DelayTime::create(.95f));
            intro.pushBack(CallFunc::create([this]() {
                if (titleAudioEnabled)
                    titleEffects.push_back(CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(
                        "web-audio/guitar.wav",false,1,0,.7f));
            }));
            intro.pushBack(DelayTime::create(.05f));
        } else intro.pushBack(DelayTime::create(1.0f));
    }
    sprite->runAction(Sequence::create(intro));
#else
	sprite->runAction(action);
#endif

	cocos2d::Animation *anim2 = Animation::create();

	char zname2[100] = { 0 };

	for (int i = 0; i < 11; i++){
		sprintf(zname2, "start2/start%i.jpg", i);
		anim2->addSpriteFrameWithFileName(zname2);
	}

	sprite2 = Sprite::create("start2/start0.jpg");
	sprite2->retain();

	anim2->setDelayPerUnit(.14f);
	auto action2 = Animate::create(anim2);
#if defined(__EMSCRIPTEN__)
    Vector<FiniteTimeAction*> neon;
    for (int frame=0; frame<11; ++frame) {
        neon.pushBack(CallFunc::create([this,frame]() {
            sprite2->setTexture("start2/start" + to_string(frame) + ".jpg");
            if (!titleAudioEnabled) return;
            auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
            if (!neonStarted) {
                neonStarted = true;
                // Guitar is cued 50 ms before the neon to compensate its lead-in.
                runAction(Sequence::create(DelayTime::create(.07f),CallFunc::create([this]() {
                    if (titleAudioEnabled)
                        titleEffects.push_back(CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(
                            "web-audio/spotlight.wav",false,1.0f,0,0.8f));
                }),nullptr));
                titleEffects.push_back(audio->playEffect("web-audio/crowd.wav",false,1,0,0.45f));
                neonBuzz = audio->playEffect("web-audio/neon-buzz.wav",true);
                titleEffects.push_back(neonBuzz);
            }
            // Measured frame brightness: 0 brightest, 5/6 dimmest, then rising.
            const float gain[] = {1,.85f,.70f,.55f,.40f,.25f,.25f,.40f,.55f,.70f,.85f};
            afroman_title_gain(neonBuzz,gain[frame]);
        }));
        neon.pushBack(DelayTime::create(.14f));
    }
    sprite2->runAction(RepeatForever::create(Sequence::create(neon)));
#else
	sprite2->runAction(RepeatForever::create(action2));
#endif

	this->schedule(schedule_selector(StartScreen::startAnim2));

	auto director = Director::getInstance();

	cameraTarget = Sprite::create();
	cameraTarget->setPosition(sprite->getPosition());

	cameraTarget->retain();

	this->addChild(cameraTarget);

	camera = Follow::create(cameraTarget, Rect::ZERO);
	camera->retain();

	this->runAction(camera);

	selectMenubg = Sprite::create("selectMenubg.png");
	selectMenubg->setOpacity(0);
	this->addChild(selectMenubg, 10000);

	return true;
}

void StartScreen::startAnim2(float delta){
	if (getActionManager()->getNumberOfRunningActionsInTarget(sprite) == 0){
		this->unscheduleAllSelectors();
		this->removeChildByName("first_anim");

		this->addChild(sprite2, 30);
	}
}

void StartScreen::selectMenu(){
#if defined(__EMSCRIPTEN__)
    stopTitleSounds();
#endif
	if (pauseMenu2) return; // Opening the existing menu again must not stack duplicate buttons.
	Vector <MenuItem*> menuItems;

	MenuItem* level1 = MenuItemImage::create("level1button.png", "level1button.png", this, menu_selector(StartScreen::selectLevel1));
	level1->setPosition(ccp(0, 125));
	menuItems.pushBack(level1);
	MenuItem* level2 = MenuItemImage::create("level2button.png", "level2button.png", this, menu_selector(StartScreen::selectLevel2));
	level2->setPosition(ccp(0, -12.5));
	menuItems.pushBack(level2);
	MenuItem* level3 = MenuItemImage::create("level3button.png", "level3button.png", this, menu_selector(StartScreen::selectLevel3));
	level3->setPosition(ccp(0, -150));
	menuItems.pushBack(level3);

	pauseMenu2 = Menu::createWithArray(menuItems);
	pauseMenu2->setPosition(cameraTarget->getPosition());
	this->addChild(pauseMenu2, 10000);

	FadeIn *fadeIn = FadeIn::create(.25);
	selectMenubg->runAction(fadeIn);
	selectMenubg->setPosition(cameraTarget->getPosition());
}

void StartScreen::selectLevel1(Ref* pSender){
	auto newScene = GameScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void StartScreen::selectLevel2(Ref* pSender){
	auto newScene = GameScene2::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void StartScreen::selectLevel3(Ref* pSender){
	auto newScene = GameScene3::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void StartScreen::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	if (keyCode == ENTER){
		selectMenu();
	}
	if (std::find(heldKeys.begin(), heldKeys.end(), keyCode) == heldKeys.end()){
		heldKeys.push_back(keyCode);
	}
}

void StartScreen::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	heldKeys.erase(std::remove(heldKeys.begin(), heldKeys.end(), keyCode), heldKeys.end());
}

void StartScreen::menuCloseCallback(Ref* pSender)
{
#if defined(__EMSCRIPTEN__)
    auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    audio->stopBackgroundMusic();
    audio->stopAllEffects();
    Director::getInstance()->replaceScene(TransitionFade::create(0.25f, StartScreen::createScene()));
    return;
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	return;
#endif

	Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}

StartScreen::StartScreen(void)
{
	setKeyboardEnabled(true);
}
StartScreen::~StartScreen(void)
{
}

#if defined(__EMSCRIPTEN__)
void StartScreen::stopTitleSounds() {
    titleAudioEnabled = false;
    auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    for (unsigned id : titleEffects) audio->stopEffect(id);
    titleEffects.clear();
    neonBuzz = 0;
}
void StartScreen::onExit() {
    stopTitleSounds();
    Layer::onExit();
}
#endif

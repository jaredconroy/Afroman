#include "StartScreen.h"
#include "globals.h"
#include "SimpleAudioEngine.h"
#include "GameScene.h"
#include "GameScene2.h"
#include "GameScene3.h"


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
	sprite->runAction(action);

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
	sprite2->runAction(RepeatForever::create(action2));

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

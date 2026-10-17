#include "GameScene3.h"
#include "StartScreen.h"
#include "globals.h"
#include "SimpleAudioEngine.h"


Scene* GameScene3::createScene()
{
	auto scene = Scene::create();

	auto layer = GameScene3::create();

	scene->addChild(layer);

	return scene;
}

bool GameScene3::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto director = Director::getInstance(); //creates the director
	wsize = director->getOpenGLView()->getFrameSize(); //finds the window size
	center = Point(wsize.width/2, wsize.height/2); //finds the coordinates of the center point

	//this is an image which serves as the background of the level
	bg_sprite = Sprite::create("level3bg.png");
	bg_sprite->setPosition(center);
	this->addChild(bg_sprite);

	//loads a tilemap which will serve as the walls
	level = new Level();
	level->loadMap("level3.tmx");
	level->getMap()->setPosition(0, 0);
	this->addChild(level->getMap());

	//creates a sprite using an image of a map point. this will serve as a destination
	point_sprite = Sprite::create("destination.png");
	point_sprite->setPosition(985, 480);
	this->addChild(point_sprite, 1000);

	//creates the player sprite
	player_sprite = Sprite::create("level3sprite.png");
	player_sprite->setPosition(261, 45);

	//creates a player based off of the player sprite
	player = new Player();
	player->position = player_sprite->getPosition();
	player->player_size = player_sprite->getBoundingBox().size;
	this->addChild(player_sprite, 100);
#if defined(__EMSCRIPTEN__)
    trail = DrawNode::create();
    this->addChild(trail, 90);
#endif
	
	//creates a player target
	cameraTarget = Sprite::create();
	cameraTarget->setPosition(center);
	this->addChild(cameraTarget);

	//creates a camera to follow the camera target
	camera = Follow::create(cameraTarget, Rect::ZERO);
	this->runAction(camera);

	this->schedule(schedule_selector(GameScene3::updateScene)); //sets the method updateScene to run every second
	
	//converts a number of seconds into minutes:seconds format
	minutes = time / 60;
	seconds = time % 60;

	//creates a label for the time remaining
	timeLabel = LabelTTF::create(to_string(minutes) + ":00", "8-Bit Madness", 50, CCSizeMake(245, 32), kCCTextAlignmentCenter);
	timeLabel->setPosition(935, 590);
	this->addChild(timeLabel, 10000);

	
	//Menus and Menu Items

	//this is a main menu which is always visible and consists only of the pause button which will open a new menu when clicked
	menu = Menu::create();
	pauseButton = MenuItemImage::create("pauseButton2.png", "pauseButtonSelected2.png", this, menu_selector(GameScene3::pauseMenu));
	menu->addChild(pauseButton);
	menu->setPosition(cameraTarget->getPositionX() + (.47 * wsize.width), cameraTarget->getPositionY() + (.42 * wsize.height));
	this->addChild(menu, 10000);

	//this is an image that serves as the background for the pause menu
	pauseMenubg = Sprite::create("pauseMenubg.png");
	pauseMenubg->setOpacity(0); //so it can fade in from 0
	this->addChild(pauseMenubg, 10000);

	//this is an image that serves as the background for the "game over" menu
	gameOverMenubg = Sprite::create("gameoverMenubg.png");
	gameOverMenubg->setOpacity(0); //so it can fade in from 0
	this->addChild(gameOverMenubg, 10000);

	//this is an image that serves as the background for the "you win" menu
	youWinMenubg = Sprite::create("youWinMenubg.png");
	youWinMenubg->setOpacity(0); //so it can fade in from 0
	this->addChild(youWinMenubg, 10000);


	tiles = level->getWalls("walls"); //creates an array of tiles from the tilemap

#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(1.0f);
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("web-audio/level3.wav", true);
#endif
	return true;
}

void GameScene3::updateScene(float delta){

	//updates the scene as long as the game is not paused
	if (!paused){
#if defined(__EMSCRIPTEN__)
        countdown.advance(time, delta);
        timeLabel->setColor(time <= 10 ? Color3B(255,35,55) : Color3B::WHITE);
        timeLabel->setScale(countdown.scale(time));
#else
		timeBySecond--; //subtract one from the time
		
		//resets the seconds count to 60 if the minute changes
		if (timeBySecond == 0){
			time--;
			timeBySecond = 60;
		}

#endif

		//calculates minutes:seconds from one number of seconds
		minutes = time / 60;
		seconds = time % 60;

		//updates what the time label says after the time has changed
		if (seconds < 10){
			timeLabel->setString(to_string(minutes) + ":0" + to_string(seconds));
		}
		else{
			timeLabel->setString(to_string(minutes) + ":" + to_string(seconds));
		}

		//Collision

		//for each tile in the tilemap, check if the player hits any of the tiles, and go to game over if it does
		for (Rect tile : tiles) {
			if (player_sprite->getBoundingBox().intersectsRect(tile)) {
				gameOverMenu();
			}
		}
		
		if (!paused) this->updatePlayer(delta);
	}
	//if time runs out before you get to the destination, go to game over
	if (time == 0){
		gameOverMenu();
	}
	//if the player gets to the destination, go to the "you win" menu
	if ((player_sprite->getPositionX() >= point_sprite->getPositionX()) && (player_sprite->getPositionY() >= point_sprite->getPositionY() - (30))){
		youWinMenu();
	}
}

void GameScene3::updatePlayer(float delta){

	//while the arrow key is held down, set the speed of the player equal to the corresponding arrow

	if (std::find(heldKeys.begin(), heldKeys.end(), UP_ARROW) != heldKeys.end()) {

		player->velocity.y = PLAYER_MAX_VELOCITY;
	}

	if (std::find(heldKeys.begin(), heldKeys.end(), RIGHT_ARROW) != heldKeys.end()){

		player->velocity.x = PLAYER_MAX_VELOCITY;
	}

	if (std::find(heldKeys.begin(), heldKeys.end(), LEFT_ARROW) != heldKeys.end()){

		player->velocity.x = -PLAYER_MAX_VELOCITY;
	}

	if (std::find(heldKeys.begin(), heldKeys.end(), DOWN_ARROW) != heldKeys.end()){

		player->velocity.y = -PLAYER_MAX_VELOCITY;
	}

	//change the position of the player by the velocity
#if defined(__EMSCRIPTEN__)
    const Vec2 previous = player->position;
#endif
	player->position = player->position + Point(player->velocity.x, player->velocity.y);
#if defined(__EMSCRIPTEN__)
    if (previous.distanceSquared(player->position) > 0)
        trail->drawSegment(previous, player->position, 3.5f, Color4F(15/255.f,117/255.f,188/255.f,0.8f));
#endif
	player_sprite->setPosition(player->position); //set the player sprite to have the same position as the player

	//reset the velocity to zero for the next time the function is called
	player->velocity = Point(0, 0);
}

void GameScene3::pauseMenu(Ref *psender){
	
	//don't pause if the game is already paused
	if (!paused){
		paused = true; //pause the game
#if defined(__EMSCRIPTEN__)
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
#endif


		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* resumeButton = MenuItemImage::create("resumeButton.png", "resumeButton.png", this, menu_selector(GameScene3::resumeGame));
		resumeButton->setPosition(ccp(0, 125));
		menuItems.pushBack(resumeButton);
		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene3::restart));
		restartButton->setPosition(ccp(0, 25));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene3::selectLevel));
		nextButton->setPosition(ccp(0, -75));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene3::menuCloseCallback));
		exitButton->setPosition(ccp(0, -175));
		menuItems.pushBack(exitButton);

		//create the menu with the menu items in the array
		pauseMenu2 = Menu::createWithArray(menuItems);
		pauseMenu2->setPosition(cameraTarget->getPosition());
		this->addChild(pauseMenu2, 10000);

		//fades in the background of the pause menu
		FadeIn *fadeIn = FadeIn::create(.25);
		pauseMenubg->runAction(fadeIn);
		pauseMenubg->setPosition(cameraTarget->getPosition());
	}
}

void GameScene3::gameOverMenu(){

	//don't pause if the game is already paused
	if (!paused){
		paused = true;
#if defined(__EMSCRIPTEN__)
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/lose.wav");
#endif


		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene3::restart));
		restartButton->setPosition(ccp(0, 125));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene3::selectLevel));
		nextButton->setPosition(ccp(0, -12.5));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene3::menuCloseCallback));
		exitButton->setPosition(ccp(0, -150));
		menuItems.pushBack(exitButton);

		//create a menu with the menu items in the array
		pauseMenu2 = Menu::createWithArray(menuItems);
		pauseMenu2->setPosition(cameraTarget->getPosition());
		this->addChild(pauseMenu2, 10000);

		//fades in the background of the game over menu
		FadeIn *fadeIn = FadeIn::create(.25);
		gameOverMenubg->runAction(fadeIn);
		gameOverMenubg->setPosition(cameraTarget->getPosition());
	}
}

void GameScene3::youWinMenu(){

	//don't pause if the game is already paused
	if (!paused){
		paused = true;
#if defined(__EMSCRIPTEN__)
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/win.wav");
#endif


		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene3::restart));
		restartButton->setPosition(ccp(0, 125));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene3::selectLevel));
		nextButton->setPosition(ccp(0, -12.5));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene3::menuCloseCallback));
		exitButton->setPosition(ccp(0, -150));
		menuItems.pushBack(exitButton);

		//create a menu with the menu items in the array
		pauseMenu2 = Menu::createWithArray(menuItems);
		pauseMenu2->setPosition(cameraTarget->getPosition());
		this->addChild(pauseMenu2, 10000);

		//fades in the background of the "you win" menu
		FadeIn *fadeIn = FadeIn::create(.5);
		youWinMenubg->runAction(fadeIn);
		youWinMenubg->setPosition(cameraTarget->getPosition());
	}
}

void GameScene3::resumeGame(Ref *psender){

	pauseMenu2->removeFromParent(); //remove the menu when the game is not pause anymore

	//fade out the background image of the menu
	FadeOut *fadeOut = FadeOut::create(.25);
	pauseMenubg->runAction(fadeOut);

	paused = false;
#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
#endif //unpause the game
}

void GameScene3::restart(Ref *psender){
	
	//creates a new scene of the same level and restarts it from the beginning
	auto newScene = GameScene3::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene3::selectLevel(Ref *psender){

	//replaces current scene with a new scene of the next level
#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();
#endif
	auto newScene = StartScreen::createScene(true);
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene3::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	if (std::find(heldKeys.begin(), heldKeys.end(), keyCode) == heldKeys.end()){
		heldKeys.push_back(keyCode);
	}
}

void GameScene3::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	heldKeys.erase(std::remove(heldKeys.begin(), heldKeys.end(), keyCode), heldKeys.end());
}

void GameScene3::menuCloseCallback(Ref* pSender)
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

GameScene3::GameScene3(void)
{
	setKeyboardEnabled(true);
}
GameScene3::~GameScene3(void)
{
}

#include "GameScene.h"
#include "StartScreen.h"
#include "globals.h"
#include "SimpleAudioEngine.h"


Scene* GameScene::createScene()
{
	auto scene = Scene::create();

	auto layer = GameScene::create();

	scene->addChild(layer);

	return scene;
}

bool GameScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	//preloads sound effect using the filename
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("powerUp.wav");

	auto director = Director::getInstance(); //creates the director
	wsize = director->getOpenGLView()->getFrameSize(); //finds the window size
	center = Point(wsize.width / 2, wsize.height / 2); //finds the coordinates of the center point

	//Tilemaps (needed to be split up because Cocos has a limit per tilemap

	//creates a level using the tilemap file and adds it to the scene
	level = new Level();
	level->loadMap("level1.tmx");
	level->retain();
	level->getMap()->setScale(SCALE_FACTOR);
	this->addChild(level->getMap());
	
	//this tilemap is just a background
	bg = new Level();
	bg->loadMap("level1bg.tmx");
	bg->retain();
	bg->getMap()->setScale(SCALE_FACTOR);
	this->addChild(bg->getMap(), -100);

	//this tilemap is in the foreground
	fg = new Level();
	fg->loadMap("level1fg.tmx");
	fg->retain();
	fg->getMap()->setScale(SCALE_FACTOR);
	this->addChild(fg->getMap(), 10);

	//add sprites and animations to cache
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("afroman1.plist");
	AnimationCache::getInstance()->addAnimationsWithFile("afroman-animations.plist");

	//create a player sprite and declare its settings like position
	player_sprite = Sprite::createWithSpriteFrameName("idle.png");
	player_sprite->setAnchorPoint(Point(0, 0));
	player_sprite->setFlippedX(false);
	player_sprite->setScale(1.0f);
	point = Point(42, 32);
	player_sprite->setPosition(level->tileCoordinateToPosition(point));
	this->addChild(player_sprite);

	//create a player based off of the player sprite
	player = new Player();
	player->retain();
	player->state = Player::State::Standing;
	player->position = player_sprite->getPosition();
	player->player_size.width = player_sprite->getBoundingBox().size.width;
	player->player_size.height = player_sprite->getBoundingBox().size.height;

	//creates a camera target and sets its position
	cameraTarget = Sprite::create();
	cameraTarget->setPositionX(player_sprite->getPosition().x + 300); // set to players x
	cameraTarget->setPositionY(2 * wsize.height); // center of height
	cameraTarget->retain();
	this->addChild(cameraTarget);

	//creates a camera which follows the camera target
	camera = Follow::create(cameraTarget, Rect::ZERO);
	camera->retain();
	this->runAction(camera);

	//sets the visible center to be the coordinates of the camera target
	center = Point(cameraTarget->getPosition());

	//creates a drumset sprite and sets its position
	drumset = Sprite::create("drumset.png");
	drumset->setScale(1.5f);
	drumset->setPosition((2 * level->getMap()->getContentSize().width) - 510, 1110);
	this->addChild(drumset, 500);

	//create a sprite called disguise. this can be picked up by the player
	disguise = Sprite::create("disguise.png");
	disguise->setPosition(ccp(3 * wsize.width, wsize.height / 1.7));
	this->addChild(disguise, 10000000);

	disguised = false; //the player is not initially disguised
	paused = false; //the game is not initially paused

	//converts a number of seconds to minutes:seconds format
	minutes = time / 60;
	seconds = time % 60;

	//creates a label with a few parameters
	timeLabel = LabelTTF::create("TIME " + to_string(minutes) + ":00", "8-Bit Madness", 40, CCSizeMake(245, 32), kCCTextAlignmentCenter);
	timeLabel->setPosition(cameraTarget->getPosition() + ccp(422, 280));
	this->addChild(timeLabel, 10000);


	//Menus and Menu Items

	//this is a main menu which is always visible and consists only of the pause button which will open a new menu when clicked
	menu = Menu::create();
	pauseButton = MenuItemImage::create("pauseButton.png", "pauseButtonSelected.png", this, menu_selector(GameScene::pauseMenu));
	menu->addChild(pauseButton);
	menu->setPosition(cameraTarget->getPositionX() - (.47 * wsize.width), cameraTarget->getPositionY() + (.45 * wsize.height));
	this->addChild(menu, 10000);

	//this image serves as the background of the pause menu
	pauseMenubg = Sprite::create("pauseMenubg.png");
	pauseMenubg->setOpacity(0); //set opacity to 0 so it can fade in from 0
	this->addChild(pauseMenubg, 10000);

	//this image serves as the background of the "game over" menu
	gameOverMenubg = Sprite::create("gameoverMenubg.png");
	gameOverMenubg->setOpacity(0); //will fade in from 0 opacity
	this->addChild(gameOverMenubg, 10000);

	//this image serves as the background of the "you win" menu
	youWinMenubg = Sprite::create("youWinMenubg.png");
	youWinMenubg->setOpacity(0); //will fade in from 0 opacity
	this->addChild(youWinMenubg, 10000);

	this->setupAnimations(); //sets up the animations in the cache

	this->schedule(schedule_selector(GameScene::updateScene)); //sets updateScene to run every second

	return true;
}

void GameScene::setupAnimations(){

	AnimationCache *cache = AnimationCache::getInstance();

	//initializes the walking animation from the cache
	Animation *animation = cache->getAnimation("walk");
	Animate* animate = Animate::create(animation);
	animate->getAnimation()->setRestoreOriginalFrame(true);
	animate->setDuration(0.80f);
	animate->setTarget(player_sprite);

	this->walkRight = animate;
	this->walkRight->retain();

	//climbing animation
	Animation *animation2 = cache->getAnimation("climb");
	Animate* animate2 = Animate::create(animation2);
	animate2->getAnimation()->setRestoreOriginalFrame(true);
	animate2->setDuration(0.80f);
	animate2->setTarget(player_sprite);

	this->climb = animate2;
	this->climb->retain();

	//disguised walking animation
	Animation *animation3 = cache->getAnimation("dwalk");
	Animate* animate3 = Animate::create(animation3);
	animate3->getAnimation()->setRestoreOriginalFrame(false);
	animate3->setDuration(0.80f);
	animate3->setTarget(player_sprite);

	this->dwalkRight = animate3;
	this->dwalkRight->retain();
}

void GameScene::updateScene(float delta){

	center = Point(cameraTarget->getPosition()); //updates the coordinates of the center point as the camera target moves

	if (!paused){ //only update if the game is not paused
		
		timeBySecond--; //decrease the time by 1 every second
		//if time by second gets to 0, reset it to 60 for the next minute;
		if (timeBySecond == 0){
			time--;
			timeBySecond = 60;
		}

		//converts number to minutes:seconds format and updates the time label to the new time
		minutes = time / 60;
		seconds = time % 60;
		if (seconds < 10){
			timeLabel->setString("TIME " + to_string(minutes) + ":0" + to_string(seconds));
		}
		else{
			timeLabel->setString("TIME " + to_string(minutes) + ":" + to_string(seconds));
		}

		//keep the time label in the same position on the screen if the camera target moves
		timeLabel->setPosition(cameraTarget->getPosition() + ccp(422, 280));

		//keep the pause button in the same position on the screen if the camera target moves
		menu->setPosition(cameraTarget->getPositionX() - (.47 * wsize.width), cameraTarget->getPositionY() + (.45 * wsize.height));

		this->updatePlayer(delta);
	}
	//if time runs out, go to game over
	if (time == 0){ 
		gameOverMenu();
	}
	//if the player reaches the destination, go to the "you win" menu
	if ((player_sprite->getPositionX() >= ((2 * level->getMap()->getContentSize().width) - wsize.height)) && (cameraTarget->getPositionY() == 1280)){
		youWinMenu();
	}
}

void GameScene::updatePlayer(float delta){

	//what it does when certain keys are pressed
	if (std::find(heldKeys.begin(), heldKeys.end(), UP_ARROW) != heldKeys.end()) {

		if (ladder){ //if the player is standing on a ladder, have him climb if up is pressed
			player->state = Player::State::Climbing;
			player->grounded = false;
			player->climbing = true;
		}
		else if (player->grounded){ //if the player is grounded, make him jump

			player->velocity.y = PLAYER_JUMP_VELOCITY;
			player->state = Player::State::Jumping;
			player->grounded = false;
		}
	}
	
	if (std::find(heldKeys.begin(), heldKeys.end(), RIGHT_ARROW) != heldKeys.end()){

		player->velocity.x = PLAYER_MAX_VELOCITY;

		if (player->grounded){
			player->state = Player::State::Walking;
		}

		player->facingRight = true;
	}

	if (std::find(heldKeys.begin(), heldKeys.end(), LEFT_ARROW) != heldKeys.end()){

		player->velocity.x = -PLAYER_MAX_VELOCITY;
		if (player->grounded){
			player->state = Player::State::Walking;
		}
		player->facingRight = false;
	}

	if (std::find(heldKeys.begin(), heldKeys.end(), DOWN_ARROW) != heldKeys.end()){

		if (sewer){ //if the player is standing on a sewer, have him climb down
			if (!player->falling){
				player->falling = true;
				player->grounded = false;
				player->state = Player::State::Climbing;
			}
		}
	}

	if ((!player->falling) && (!player->climbing)){ 
		player->velocity -= Point(0, GRAVITY);
	}
	else{
		player->velocity.x = 0;
		player_sprite->setFlippedX(false);
	}

	this->getCollisionTiles(delta); //gets the collidable tiles

	if (slow){
		PLAYER_MAX_VELOCITY = 5.0f;
	}
	else{
		PLAYER_MAX_VELOCITY = 8.0F;
	}

	// change the player position based on its velocity

	player->position.x = player->position.x + player->velocity.x;
	if (player_sprite->getPosition().x + 300 <= ((level->getMap()->getContentSize().width * 2) - (wsize.width/2)))
	{
		cameraTarget->setPositionX(player_sprite->getPosition().x + 300);
	}

	player->position.y = player->position.y + player->velocity.y;
	if (player->falling){
		cameraTarget->setPositionY(cameraTarget->getPositionY() - 15);
		player->velocity.y = -15;
	}
	if (player->climbing){
		if (player->position.y < (level->tileCoordinateToPosition(point).y)){
			cameraTarget->setPositionY(cameraTarget->getPositionY() + 15);
			player->velocity.y = 15;
		}
		else{
			player->velocity.y = 0;
			if (cameraTarget->getPositionY() == 1280){
				player->climbing = false;
				player->falling = false;
			}
			else{
				cameraTarget->setPositionY(cameraTarget->getPositionY() + 15);
			}
		}
	}

	this->updatePlayerSprite(delta);

	if (std::find(heldKeys.begin(), heldKeys.end(), RIGHT_ARROW) == heldKeys.end() && player->grounded){
		player->velocity.x = 0;
		player->state = Player::State::Standing;
	}

	if (std::find(heldKeys.begin(), heldKeys.end(), LEFT_ARROW) == heldKeys.end() && player->grounded){
		player->velocity.x = 0;
		player->state = Player::State::Standing;
	}

}

void GameScene::updatePlayerSprite(float delta){

	//sets the sprite up to do the proper animation

	if (player->state == Player::State::Walking){
		//if disguised, use walk animation with disguise. else, use the regular animation
		if (disguised){
			if (dwalkRight->isDone()){
				dwalkRight->startWithTarget(player_sprite);
			}

			dwalkRight->step(delta);
		}
		else{
			if (walkRight->isDone()){
				walkRight->startWithTarget(player_sprite);
			}

			walkRight->step(delta);
		}

		//flip the image depending on which way the player is facing
		if (player->facingRight){
			player_sprite->setFlippedX(false);
		}
		else {
			player_sprite->setFlippedX(true);
		}

	}
	else if (player->state == Player::State::Climbing){
		//do the climb animation
		if (climb->isDone()){
			climb->startWithTarget(player_sprite);
		}

		climb->step(delta);
	}

	else if (player->state == Player::State::Jumping){
		if (disguised){
			player_sprite->setSpriteFrame(Sprite::createWithSpriteFrameName("djump.png")->getSpriteFrame());
		}
		else{
			player_sprite->setSpriteFrame(Sprite::createWithSpriteFrameName("jump.png")->getSpriteFrame());
		}
		
		if (player->facingRight){
			player_sprite->setFlippedX(false);
		}
		else {
			player_sprite->setFlippedX(true);
		}

	}
	else{
		if (disguised){
			player_sprite->setSpriteFrame(Sprite::createWithSpriteFrameName("didle.png")->getSpriteFrame());
		}
		else{
			player_sprite->setSpriteFrame(Sprite::createWithSpriteFrameName("idle.png")->getSpriteFrame());
		}
	}

	player_sprite->setPosition(player->position); //set the player sprite position equal to the player's position
}

void GameScene::getCollisionTiles(float delta){

	Point tmp;
	Point corner;
	vector<Rect> tiles;
	vector<Rect> corners;

	if (player->facingRight){ //collide into the bottom right corner, or maximum x. Do inverse for other corner
		tmp = level->positionToTileCoordinate(Point(player->position.x + player->player_size.width, player->position.y));
		corner = level->positionToTileCoordinate(Point(player->position.x + (3 * player->player_size.width / 7), player->position.y));
	}
	else { //collide into the bottom left corner, or minimum x. Do inverse for other corner
		tmp = level->positionToTileCoordinate(Point(player->position.x, player->position.y));
		corner = level->positionToTileCoordinate(Point(player->position.x + (4 * player->player_size.width / 7), player->position.y));
	}

	Rect player_rect;

	if (player->facingRight){
		player_rect.setRect(player_sprite->getBoundingBox().getMinX() + player->velocity.x, player_sprite->getBoundingBox().getMinY(),
			3 * player->player_size.width / 4, player->player_size.height);
	}
	else {
		player_rect.setRect(player_sprite->getBoundingBox().getMinX() + player->velocity.x + (player->player_size.width / 4),
			player_sprite->getBoundingBox().getMinY(), 3 * player->player_size.width / 4, player->player_size.height);
	}

	//if the player walks into the disguise, make the player disguised and play the sound
	if (player_rect.intersectsRect(disguise->getBoundingBox()) && disguise->isVisible()){
		disguised = true;
		disguise->setVisible(false);
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("powerUp.wav");
	}

	// Wall collision

	tiles = level->getTiles(tmp, -1, 0, "walls");

	//for each tile, if the player walks into the tile, set the velocity to 0
	for (Rect tile : tiles) {
		if (player_rect.intersectsRect(tile)) {
			player->velocity.x = 0;
			if (player->facingRight){
				player->position.x = tile.getMinX() - (3 * player->player_size.width / 4) - 2;
			}
			else{
				player->position.x = tile.getMaxX() - (player->player_size.width / 4) + 2;
			}
			break;
		}
	}

	tiles.clear();

	// Ground collision

	tiles = level->getTiles(tmp, -1, -1, "walls");
	corners = level->getTiles(corner, -1, -1, "walls");

	if (player->facingRight){
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x,
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}
	else {
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x + (player->player_size.width / 4),
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}

	for (Rect tile : tiles) {
		player->grounded = false;
		if (tile.intersectsRect(player_rect)) {

			player->position.y = tile.getMaxY();
			// if we hit the ground, mark us as grounded so we can jump
			player->grounded = true;
			player->falling = false;
			sewer = false;

			player->velocity.y = 0;
			break;
		}
	}

	for (Rect tile : corners) {
		player->grounded = false;
		if (tile.intersectsRect(player_rect)) {

			player->position.y = tile.getMaxY();
			// if we hit the ground, mark us as grounded so we can jump
			player->grounded = true;
			player->falling = false;
			sewer = false;

			player->velocity.y = 0;
			break;
		}
	}

	tiles.clear();
	corners.clear();

	// Sewer collision

	tiles = level->getTiles(tmp, -1, -1, "sewer");
	corners = level->getTiles(corner, -1, -1, "sewer");

	if (player->facingRight){
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x,
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}
	else {
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x + (player->player_size.width / 4),
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}

	for (Rect tile : tiles) {
		sewer = false;
		player->grounded = false;
		if (tile.intersectsRect(player_rect)) {
			sewer = true;
			if (!player->falling){
				player->position.y = tile.getMaxY();
				player->grounded = true;
				player->velocity.y = 0;
			}
			break;
		}
	}

	for (Rect tile : corners) {
		sewer = false;
		player->grounded = false;
		if (tile.intersectsRect(player_rect)) {
			sewer = true;
			if (!player->falling){
				player->position.y = tile.getMaxY();
				player->grounded = true;
				player->velocity.y = 0;
			}
			break;
		}
	}

	tiles.clear();
	corners.clear();

	// Hazard collision

	tiles = level->getTiles(tmp, -1, -1, "hazards");
	corners = level->getTiles(corner, -1, -1, "hazards");

	if (player->facingRight){
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x,
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}
	else {
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x + (player->player_size.width / 4),
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}

	slow = false;

	for (Rect tile : tiles) {
		if (tile.intersectsRect(player_rect)) {
			if (cameraTarget->getPositionY() == 1280){
				if (!disguised){
					slow = true;
					break;
				}
			}
			else{
				slow = true;
				break;
			}
		}
	}

	for (Rect tile : corners) {
		if (tile.intersectsRect(player_rect)) {
			if (cameraTarget->getPositionY() == 1280){
				if (!disguised){
					slow = true;
					break;
				}
			}
			else{
				slow = true;
				break;
			}
		}
	}

	tiles.clear();
	corners.clear();

	// Ladder collision

	tiles = level->getTiles(tmp, -1, -1, "ladder");
	corners = level->getTiles(corner, -1, -1, "ladder");

	if (player->facingRight){
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x,
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}
	else {
		player_rect.setRect(
			player_sprite->getBoundingBox().getMinX() + player->velocity.x + (player->player_size.width / 4),
			player_sprite->getBoundingBox().getMinY() + player->velocity.y,
			3 * player->player_size.width / 4,
			player->player_size.height
			);
	}

	ladder = false;

	for (Rect tile : tiles) {
		if (tile.intersectsRect(player_rect)) {
			ladder = true;
			break;
		}
	}

	for (Rect tile : corners) {
		if (tile.intersectsRect(player_rect)) {
			ladder = true;
			break;
		}
	}

	tiles.clear();
	corners.clear();
}

void GameScene::pauseMenu(Ref *psender){
	
	//don't pause if the game is already paused
	if (!paused){
		paused = true; //pause the game

		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* resumeButton = MenuItemImage::create("resumeButton.png", "resumeButton.png", this, menu_selector(GameScene::resumeGame));
		resumeButton->setPosition(ccp(0, 125));
		menuItems.pushBack(resumeButton);
		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene::restart));
		restartButton->setPosition(ccp(0, 25));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene::selectLevel));
		nextButton->setPosition(ccp(0, -75));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene::menuCloseCallback));
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

void GameScene::gameOverMenu(){

	//don't pause if the game is already paused
	if (!paused){
		paused = true; //pause the game

		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene::restart));
		restartButton->setPosition(ccp(0, 125));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene::selectLevel));
		nextButton->setPosition(ccp(0, -12.5));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene::menuCloseCallback));
		exitButton->setPosition(ccp(0, -150));
		menuItems.pushBack(exitButton);

		//create the menu with the menu items in the array
		pauseMenu2 = Menu::createWithArray(menuItems);
		pauseMenu2->setPosition(cameraTarget->getPosition());
		this->addChild(pauseMenu2, 10000);

		//fades in the background of the game over menu
		FadeIn *fadeIn = FadeIn::create(.25);
		gameOverMenubg->runAction(fadeIn);
		gameOverMenubg->setPosition(cameraTarget->getPosition());
	}
}

void GameScene::youWinMenu(){

	//don't pause if the game is already paused
	if (!paused){
		paused = true; //pause the game

		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene::restart));
		restartButton->setPosition(ccp(0, 125));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene::selectLevel));
		nextButton->setPosition(ccp(0, -12.5));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene::menuCloseCallback));
		exitButton->setPosition(ccp(0, -150));
		menuItems.pushBack(exitButton);

		//create the menu with the menu items in the array
		pauseMenu2 = Menu::createWithArray(menuItems);
		pauseMenu2->setPosition(cameraTarget->getPosition());
		this->addChild(pauseMenu2, 10000);

		//fades in the background of the "you win" menu
		FadeIn *fadeIn = FadeIn::create(1);
		youWinMenubg->runAction(fadeIn);
		youWinMenubg->setPosition(cameraTarget->getPosition());
	}
}

void GameScene::resumeGame(Ref *psender){
	pauseMenu2->removeFromParent();

	FadeOut *fadeOut = FadeOut::create(.25);
	pauseMenubg->runAction(fadeOut);
	paused = false;
}

void GameScene::restart(Ref *psender){
	
	//creates a new scene of the same level and restarts it from the beginning
	auto newScene = GameScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene::selectLevel(Ref *psender){
	
	//replaces current scene with a new scene of the next level
	auto newScene = StartScreen::createScene(true);
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	if (std::find(heldKeys.begin(), heldKeys.end(), keyCode) == heldKeys.end()){
		heldKeys.push_back(keyCode);
	}
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	heldKeys.erase(std::remove(heldKeys.begin(), heldKeys.end(), keyCode), heldKeys.end());
}

void GameScene::menuCloseCallback(Ref* pSender)
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

GameScene::GameScene(void)
{
	setKeyboardEnabled(true);
	setTouchEnabled(true);
}
GameScene::~GameScene(void)
{
}

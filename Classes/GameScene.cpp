#include "GameScene.h"
#include "StartScreen.h"
#include "globals.h"
#include "SimpleAudioEngine.h"


#if defined(__EMSCRIPTEN__)
namespace {
void drawLightRect(DrawNode* node, Vec2 a, Vec2 b, const Color4F& color) {
    Vec2 corners[] = {a,Vec2(b.x,a.y),b,Vec2(a.x,b.y)};
    node->drawPolygon(corners,4,color,0,Color4F(0,0,0,0));
}
float triggerCenter(Level* level, const char* name, const Rect& tile) {
    auto map = level->getMap();
    auto layer = map->getLayer(name);
    const float width = map->getTileSize().width * SCALE_FACTOR;
    const float height = map->getTileSize().height * SCALE_FACTOR;
    int left = static_cast<int>(tile.getMidX() / width), right = left;
    int row = static_cast<int>(map->getMapSize().height) - 1 - static_cast<int>(tile.getMidY() / height);
    while (left > 0 && layer->getTileGIDAt(Vec2(left-1,row))) --left;
    while (right+1 < map->getMapSize().width && layer->getTileGIDAt(Vec2(right+1,row))) ++right;
    return (left+right+1) * width * 0.5f;
}
}
#endif

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
#if defined(__EMSCRIPTEN__)
    // Display the authored bin as one sprite so only its art shakes; TMX collision
    // tiles remain present. Body is walls (175..179,12..17), lid is foreground row 11.
    for (int x=175; x<=179; ++x) {
        for (int y=12; y<=17; ++y)
            if (auto tile = level->getMap()->getLayer("walls")->getTileAt(Vec2(x,y))) tile->setOpacity(0);
        if (auto lid = fg->getMap()->getLayer("foreground")->getTileAt(Vec2(x,11))) lid->setOpacity(0);
    }
    trashBin = Sprite::create("tileSet.png",Rect(177,145,80,112));
    trashBin->setAnchorPoint(Vec2(.5f,0));
    trashBin->setScale(2);
    trashBin->setPosition(5680,1024);
    this->addChild(trashBin,11);
    // Headlamp on the authored blue car's front bumper (TMX column 155).
    carLights = DrawNode::create();
    // Existing white lamp pixels: native x2484..2495, y216..227.
    // Beam starts at their right edge, centered at world y1156; no extra lamp art.
    Vec2 beam[] = {Vec2(4992,1144),Vec2(5600,1056),
                   Vec2(5600,1256),Vec2(4992,1168)};
    carLights->drawPolygon(beam,4,Color4F(1,1,.8f,.22f),0,Color4F(0,0,0,0));
    carLights->setVisible(false);
    this->addChild(carLights,12);
    // Authored bathroom interior: native TMX x=208..368, y=0..288, at 2x scale.
    bathroomShade = LayerColor::create(Color4B(0,0,0,115),320,576);
    bathroomShade->setPosition(416,1024);
    this->addChild(bathroomShade,-50);
    bathroomWindowLight = LayerColor::create(Color4B(255,255,255,0),146,212);
    bathroomWindowLight->setPosition(406,1206);
    this->addChild(bathroomWindowLight,-45);
    bathroomLamp = DrawNode::create();
    this->addChild(bathroomLamp,-40);
    drawLightRect(bathroomLamp,Vec2(562,1512),Vec2(590,1520),Color4F(.28f,.28f,.28f,1));
    drawLightRect(bathroomLamp,Vec2(568,1506),Vec2(584,1512),Color4F(.48f,.46f,.38f,1));
    // Find the left wall on the spawn row without materializing the whole TMX layer.
    auto walls = level->getMap()->getLayer("walls");
    const float tileWidth = level->getMap()->getTileSize().width * SCALE_FACTOR;
    const float tileHeight = level->getMap()->getTileSize().height * SCALE_FACTOR;
    const int row = static_cast<int>(level->getMap()->getMapSize().height) - 1 -
        static_cast<int>(player->position.y / tileHeight);
    for (int x = 0; (x+1)*tileWidth <= player->position.x; ++x)
        if (walls->getTileGIDAt(Vec2(x,row))) flushBoundaryX = (x+1)*tileWidth;
#endif

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

#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(1.0f);
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("web-audio/level1.wav", true);
#endif
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
		
#if defined(__EMSCRIPTEN__)
        countdown.advance(time, delta);
        timeLabel->setColor(time <= 10 ? Color3B(255,35,55) : Color3B::WHITE);
        timeLabel->setScale(countdown.scale(time));
#else
		timeBySecond--; //decrease the time by 1 every second
		//if time by second gets to 0, reset it to 60 for the next minute;
		if (timeBySecond == 0){
			time--;
			timeBySecond = 60;
		}

#endif

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
#if defined(__EMSCRIPTEN__)
        updateBathroomLight();
#endif
	}
	//if time runs out, go to game over
	if (time == 0){ 
		gameOverMenu();
	}
	//if the player reaches the destination, go to the "you win" menu
	if (time > 0 && (player_sprite->getPositionX() >= ((2 * level->getMap()->getContentSize().width) - wsize.height)) && (cameraTarget->getPositionY() == 1280)){
		youWinMenu();
	}
}

void GameScene::updatePlayer(float delta){
#if defined(__EMSCRIPTEN__)

#endif

	//what it does when certain keys are pressed
	if (std::find(heldKeys.begin(), heldKeys.end(), UP_ARROW) != heldKeys.end()) {

		if (ladder){ //if the player is standing on a ladder, have him climb if up is pressed
#if defined(__EMSCRIPTEN__)
            if (!player->climbing) {
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/ladder-up.wav",false,1,0,.65f);
                player->position.x = ladderCenterX - player->player_size.width * 0.5f;
                player_sprite->setPosition(player->position);
            }
#endif
			player->state = Player::State::Climbing;
			player->grounded = false;
			player->climbing = true;
		}
		else if (player->grounded){ //if the player is grounded, make him jump

#if defined(__EMSCRIPTEN__)
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/jump.wav");
#endif
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
#if defined(__EMSCRIPTEN__)
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/ladder-down.wav",false,1,0,.65f);
                player->position.x = sewerCenterX - player->player_size.width * 0.5f;
                player_sprite->setPosition(player->position);
#endif
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

#if defined(__EMSCRIPTEN__)
    bool flushContact = false;
    bool binContact = false, carContact = false, landed = false;
    const float incomingY = player->velocity.y;
    const float feetBefore = player->position.y;
    const float leftBefore = player->position.x + (player->facingRight ? 0 : player->player_size.width*.25f);
    auto binTile = [](const Rect& tile) {
        return tile.getMinX() >= 5600 && tile.getMaxX() <= 5760 &&
            tile.getMinY() >= 1024 && tile.getMaxY() <= 1216;
    };
    auto groundContact = [&](const Rect& tile) {
        if (incomingY > 0 || feetBefore < tile.getMaxY()-2) return;
        landed = true;

        carContact = carContact || (tile.getMinX() >= 4384 && tile.getMaxX() <= 4992 &&
            tile.getMinY() >= 1088 && tile.getMaxY() <= 1280);
    };
#endif
	// Wall collision

	tiles = level->getTiles(tmp, -1, 0, "walls");

	//for each tile, if the player walks into the tile, set the velocity to 0
	for (Rect tile : tiles) {
		if (player_rect.intersectsRect(tile)) {
#if defined(__EMSCRIPTEN__)
            // Side impacts must originate outside the bin, so walking off its
            // top cannot become a new side impact during the following fall.
            if (binTile(tile) && feetBefore < 1214 &&
                ((player->velocity.x > 0 && leftBefore+player->player_size.width*.75f <= 5602) ||
                 (player->velocity.x < 0 && leftBefore >= 5758)))
                binContact = true;
            flushContact = !player->facingRight && player->velocity.x < 0 &&
                tile.getMaxX() <= flushBoundaryX + 0.1f && cameraTarget->getPositionY() == 1280;
#endif
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

#if defined(__EMSCRIPTEN__)
    if (flushContact && !touchingFlushWall)
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/flush.wav");
    touchingFlushWall = flushContact;
#endif
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

#if defined(__EMSCRIPTEN__)
            groundContact(tile);
#endif
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

#if defined(__EMSCRIPTEN__)
            groundContact(tile);
#endif
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

#if defined(__EMSCRIPTEN__)
    // A swept top-plane test handles gentle landings and tile-corner misses.
    const float leftAfter = player->position.x + player->velocity.x +
        (player->facingRight ? 0 : player->player_size.width*.25f);
    const bool overBin = leftAfter < 5760 && leftAfter+player->player_size.width*.75f > 5600;
    const bool crossedTop = overBin && incomingY <= 0 && feetBefore >= 1216 &&
        feetBefore+incomingY <= 1216;
    if (crossedTop) {
        player->position.y = 1216;
        player->velocity.y = 0;
        player->grounded = true;
        player->falling = false;
        landed = true;
    }
    const bool onBinTop = overBin && incomingY <= 0 &&
        std::abs(player->position.y-1216) <= 2;
    if (binTopImpact.update(onBinTop,overBin,player->position.y)) hitTrashBin();
    if (binContact && !touchingTrashBin && !onBinTop) hitTrashBin();
    if (carContact && !touchingCar) {
        carLightsOn = !carLightsOn;
        carLights->setVisible(carLightsOn);
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/car-horn.wav",false,1,0,.65f);
    }
    if (landed && incomingY < -5 && !onBinTop)
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/land.wav",false,1,0,.5625f);
    touchingTrashBin = binContact;
    // Ignore one-frame holes in legacy support detection while walking on the car.
    if (carContact) touchingCar = true;
    else if (incomingY > 0 || leftAfter >= 4992 || leftAfter+player->player_size.width*.75f <= 4384)
        touchingCar = false;
#endif
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
#if defined(__EMSCRIPTEN__)
            sewerCenterX = triggerCenter(level, "sewer", tile);
#endif
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
#if defined(__EMSCRIPTEN__)
            sewerCenterX = triggerCenter(level, "sewer", tile);
#endif
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
#if defined(__EMSCRIPTEN__)
            ladderCenterX = triggerCenter(level, "ladder", tile);
#endif
			break;
		}
	}

	for (Rect tile : corners) {
		if (tile.intersectsRect(player_rect)) {
			ladder = true;
#if defined(__EMSCRIPTEN__)
            ladderCenterX = triggerCenter(level, "ladder", tile);
#endif
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
#if defined(__EMSCRIPTEN__)
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
#endif


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
#if defined(__EMSCRIPTEN__)
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/lose.wav");
#endif


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
#if defined(__EMSCRIPTEN__)
        CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/win.wav");
#endif


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
#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
#endif
}

void GameScene::restart(Ref *psender){
	
	//creates a new scene of the same level and restarts it from the beginning
	auto newScene = GameScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene::selectLevel(Ref *psender){
	
	//replaces current scene with a new scene of the next level
#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();
#endif
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

#if defined(__EMSCRIPTEN__)
void GameScene::updateBathroomLight() {
    const float centerX = player->position.x + player->player_size.width * 0.5f;
    // A small doorway hysteresis prevents rapid clicks while straddling its edge.
    const bool entered = centerX >= 416 && centerX < (bathroomOccupied ? 752 : 736) &&
        player->position.y >= 1020 && player->position.y < 1600;
    if (entered == bathroomOccupied) return;
    bathroomOccupied = entered;
    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("web-audio/lightswitch.wav");
    bathroomShade->stopAllActions();
    bathroomShade->runAction(FadeTo::create(.12f,entered ? 0 : 115));
    bathroomWindowLight->stopAllActions();
    bathroomWindowLight->runAction(FadeTo::create(.12f,entered ? 175 : 0));
    bathroomLamp->clear();
    drawLightRect(bathroomLamp,Vec2(562,1512),Vec2(590,1520),Color4F(.28f,.28f,.28f,1));
    drawLightRect(bathroomLamp,Vec2(568,1506),Vec2(584,1512),
        entered ? Color4F(1,1,1,1) : Color4F(.48f,.46f,.38f,1));
}
#endif

#if defined(__EMSCRIPTEN__)
void GameScene::hitTrashBin() {
    auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    audio->playEffect("web-audio/trash-metal.wav",false,1,0,.85f);
    audio->playEffect("web-audio/trash-slosh.wav",false,1,0,.65f);
    trashBin->stopAllActions();
    trashBin->setPosition(5680,1024);
    trashBin->setRotation(0);
    trashBin->runAction(Sequence::create(RotateTo::create(.07f,-5),RotateTo::create(.1f,5),
        RotateTo::create(.1f,-3),RotateTo::create(.12f,0),nullptr));
    trashBin->runAction(Sequence::create(MoveBy::create(.06f,Vec2(-6,0)),
        MoveBy::create(.1f,Vec2(12,0)),MoveBy::create(.1f,Vec2(-9,0)),
        MoveBy::create(.12f,Vec2(3,0)),nullptr));
    const Color4B colors[] = {Color4B(72,130,60,255),Color4B(230,219,173,255),
        Color4B(160,43,51,255),Color4B(103,153,77,255)};
    const float drift[] = {-125,-95,105,140};
    for (int i=0;i<4;++i) {
        auto scrap = LayerColor::create(colors[i],8,5);
        scrap->setPosition(5680+(i-2)*8,1240);
        this->addChild(scrap,12);
        scrap->runAction(JumpBy::create(.55f,Vec2(drift[i],-212),28+i*5,1));
        scrap->runAction(Sequence::create(DelayTime::create(.35f),FadeOut::create(.2f),
            RemoveSelf::create(),nullptr));
    }
}
#endif

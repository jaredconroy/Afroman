#include "GameScene2.h"
#include "StartScreen.h"
#include "globals.h"
#include "SimpleAudioEngine.h"
#if defined(__EMSCRIPTEN__)
#include "WebRhythm.h"
#endif


Scene* GameScene2::createScene()
{
	auto scene = Scene::create();

	auto layer = GameScene2::create();

	scene->addChild(layer);

	return scene;
}

bool GameScene2::init()
{
	if (!Layer::init())
	{
		return false;
	}

	//preload the sound of each drum

	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("hihat.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("snare.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("bass.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("cymbal.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("tom1.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("tom2.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("tom3.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("error.wav");

	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic("song.mp3"); //preload the background music file

	auto director = Director::getInstance(); //creates the director

	wsize = director->getOpenGLView()->getFrameSize(); //finds the size of the window
	center = Point(wsize.width / 2, wsize.height / 2); //finds the coordinates of the center point

	//tilemap of the notes

	level = new Level(); //creates a new level called "level"
	level->loadMap("level2.tmx"); //loads the map
	level->getMap()->setAnchorPoint(Point(0, 0)); //sets the point that determines the position to be the bottom-lefthand corner
	level->getMap()->setPosition((-3500) - level->getMap()->getContentSize().width, 0); //sets the initial position of the map
	this->addChild(level->getMap(), 10); //adds the tilemap to the scene

	//creates the score label with many parameters
	scoreLabel1 = LabelTTF::create("SCORE: " + to_string(score), "8-Bit Madness", 40, CCSizeMake(245, 32), kCCTextAlignmentCenter);
	scoreLabel1->setPosition(422, 600); //sets position of the label
	this->addChild(scoreLabel1, 10000); //adds label to the scene

	//the background image as a sprite
	bg_sprite = Sprite::create("level2bg.png"); //creates the sprite using the filename
	bg_sprite->setPosition(0, center.y); //sets the position
	this->addChild(bg_sprite, 0); //adds the sprite to the scene

	//creates the camera
	cameraTarget = Sprite::create(); //creates the target as a sprite
	cameraTarget->setPosition(bg_sprite->getPosition()); //sets the position of the target
	this->addChild(cameraTarget); //adds the camera target to the scene
	camera = Follow::create(cameraTarget, Rect::ZERO); //camera will follow the camera target
	this->runAction(camera);


	//Menus and Menu Items

	//this is a main menu which is always visible and consists only of the pause button which will open a new menu when clicked
	menu = Menu::create();
	pauseButton = MenuItemImage::create("pauseButton.png", "pauseButtonSelected.png", this, menu_selector(GameScene2::pauseMenu));
	menu->addChild(pauseButton);
	menu->setPosition(cameraTarget->getPositionX() - (.47 * wsize.width), cameraTarget->getPositionY() + (.45 * wsize.height));
	this->addChild(menu, 10000);

	//this is an image showing the controls of this level
	controls = Sprite::create("controls.png");
	controls->setPosition(cameraTarget->getPosition());
	this->addChild(controls, 10000);
	
	//this is an image that serves as the background of the pause menu
	pauseMenubg = Sprite::create("pauseMenubg.png");
	pauseMenubg->setOpacity(0); //sets the opacity to 0 so that it will fade in from 0
	this->addChild(pauseMenubg, 10000);

	//this is an image that serves as the background of the "you win" menu
	youWinMenubg = Sprite::create("score.png");
	youWinMenubg->setPosition(cameraTarget->getPosition());
	youWinMenubg->setOpacity(0); //sets the opacity to 0 so that it will fade in from 0
	this->addChild(youWinMenubg, 10000);

	//creates another score label that is invisible and will only become visible at when the level is over and the "you win" menu is visible
	scoreLabel2 = LabelTTF::create(to_string(score), "8-Bit Madness", 60, CCSizeMake(500, 40), kCCTextAlignmentCenter);
	scoreLabel2->setPosition(cameraTarget->getPosition() + ccp(0, 155));
	scoreLabel2->setVisible(false);
	this->addChild(scoreLabel2, 10000);


#if defined(__EMSCRIPTEN__)
    rhythm.originX = level->getMap()->getPositionX();
    rhythm.targetX = center.x - 60.0; // center of the on-screen key boxes
    rhythm.tileWidth = level->getMap()->getTileSize().width;
    const char* lanes[] = {"cymbal", "hihat", "snare", "tom1", "tom2", "tom3", "bass"};
    for (int lane=0; lane<7; ++lane)
        for (int column:level->getNotes(lanes[lane], (lane+1)*2))
            rhythmNotes[lane].emplace_back(column);
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("song.mp3", false);
#else
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("song.mp3", true);
#endif //starts playing the background music

	this->schedule(schedule_selector(GameScene2::updateScene)); //sets the method updateScene to run every second

	return true;
}

#if defined(__EMSCRIPTEN__)
namespace {
// Rectangles use LayerColor, whose opacity actions work in Cocos 3.2.
// DrawNode stores alpha in its vertex colors and does not inherit a FadeOut.
void addFeedbackRing(Node* parent, float side, float thickness, const Color4B& color, int z) {
    const float half = side * 0.5f;
    const Rect edges[] = {
        Rect(-half, -half, side, thickness),
        Rect(-half, half-thickness, side, thickness),
        Rect(-half, -half+thickness, thickness, side-2*thickness),
        Rect(half-thickness, -half+thickness, thickness, side-2*thickness)
    };
    for (const auto& edge:edges) {
        auto strip = LayerColor::create(color, edge.size.width, edge.size.height);
        strip->setPosition(edge.origin);
        parent->addChild(strip, z);
    }
}
void addFeedbackBorder(Node* parent, const Color4B& color) {
    addFeedbackRing(parent, 54, 4, Color4B(color.r,color.g,color.b,65), 1);
    addFeedbackRing(parent, 50, 2.5f, color, 2);
}
}

void GameScene2::showRhythmFeedback(int lane, bool hit, bool practice, int column) {
    const Color4B color = practice ? Color4B(235,245,255,255) :
        (hit ? Color4B(40,255,105,255) : Color4B(255,40,65,255));
    const int feedbackTag = 71000 + lane;
    if (auto previous = this->getChildByTag(feedbackTag)) previous->removeFromParent();
    auto receptor = Node::create();
    receptor->setCascadeOpacityEnabled(true);
    // level2bg.png's white squares occupy x=966..1015, y=75+80*lane..124+80*lane.
    // Transform the image coordinates into the same world space as the chart.
    const Vec2 imageOrigin = bg_sprite->getPosition() - Vec2(525,320);
    receptor->setPosition(imageOrigin + Vec2(991,540-80*lane));
    this->addChild(receptor, 100, feedbackTag);

    auto key = Sprite::create("level2bg.png", Rect(966,75+80*lane,50,50));
    key->setBlendFunc(BlendFunc::ADDITIVE); // black pixels must not obscure the flashing note
    key->setOpacity(175);
    receptor->addChild(key, 0);
    key->runAction(Sequence::create(
        EaseOut::create(ScaleTo::create(0.045f,1.12f),2.0f),
        EaseIn::create(ScaleTo::create(0.14f,1.0f),2.0f), nullptr));
    addFeedbackBorder(receptor, color);
    receptor->runAction(Sequence::create(DelayTime::create(0.055f),FadeOut::create(0.25f),RemoveSelf::create(),nullptr));

    if (column < 0 || practice) return;
    const char* lanes[] = {"cymbal","hihat","snare","tom1","tom2","tom3","bass"};
    auto layer = level->getMap()->getLayer(lanes[lane]);
    auto tile = layer->getTileAt(Vec2(column,(lane+1)*2));
    if (!tile) return;
    const std::string name = "note-feedback-" + to_string(lane) + "-" + to_string(column);
    if (auto previous = level->getMap()->getChildByName(name)) previous->removeFromParent();
    auto note = Node::create();
    note->setCascadeOpacityEnabled(true);
    note->setPosition(layer->getPosition()+tile->getPosition()+Vec2(20,20));
    level->getMap()->addChild(note,100,name);

    // The tileset has alpha 166/255. An opaque RGB overlay flashes to full color
    // without modifying its shared texture or the timing/scoring state.
    const Color4B palette[] = {
        Color4B(255,222,22,255), Color4B(32,64,154,255), Color4B(237,28,36,255),
        Color4B(20,167,81,255), Color4B(104,42,122,255), Color4B(241,96,37,255),
        Color4B(37,170,225,255)
    };
    auto fill = LayerColor::create(palette[lane],40,40);
    fill->setPosition(-20,-20);
    note->addChild(fill);
    addFeedbackBorder(note,color);
    note->setOpacity(0);
    note->runAction(Sequence::create(FadeIn::create(0.025f),DelayTime::create(0.04f),
        FadeOut::create(0.23f),RemoveSelf::create(),nullptr));
}

void GameScene2::updateRhythm() {
    if (paused) return;
    const double seconds = afroman::musicTime();
    const double offset = afroman::calibrationOffset();
    level->getMap()->setPositionX(rhythm.position(seconds, offset));
    // Re-evaluate at keydown, not from a potentially stale rendered frame.
    if (!controlsDismissed && level->getMap()->getPositionX() >=
        -center.x - level->getMap()->getContentSize().width) {
        controls->stopAllActions();
        controls->runAction(FadeOut::create(0.25f));
        controlsDismissed = true;
    }
    scoreLabel1->setString("SCORE: " + to_string(score));
    if (level->getMap()->getPositionX() >= center.x) youWinMenu();
}
#endif

void GameScene2::updateScene(float delta){
#if defined(__EMSCRIPTEN__)
    updateRhythm();
    return;
#endif


	//if the first note is entering the screen, fade the controls image out
	if (level->getMap()->getPositionX() <= ((-center.x) - level->getMap()->getContentSize().width)){
		FadeOut* fadeOut = FadeOut::create(.25);
		controls->runAction(fadeOut);
	}

	//if the level is over, display the "you win" menu
	if (level->getMap()->getPositionX() >= (center.x)){
		youWinMenu();
	}

	//if the level is not paused, update the scene
	if (!paused){
		scoreLabel1->setString("SCORE: " + to_string(score)); //update the score to show the current score

		level->getMap()->setPositionX(level->getMap()->getPositionX() + (8.2)); //move the map each update for the notes to move across the screen


		//checking to see if there are notes at the position where they need to be hit for each drum

		string drums[] = { "cymbal", "hihat", "snare", "tom1", "tom2", "tom3", "bass" };

		bool *bools[] = { &cymbal, &hihat, &snare, &tom1, &tom2, &tom3, &bass };

		for (int i = 0; i < 7; i++){
			int k = (i + 1) * 2;
			for (int j : level->getNotes(drums[i], k)){
				if ((level->getMap()->getPositionX() <= (center.x + 5 - level->getMap()->getTileSize().width - (j * level->getMap()->getTileSize().width))) &&
					(level->getMap()->getPositionX() >= (center.x - 5 - (j * level->getMap()->getTileSize().width) - (3 * level->getMap()->getTileSize().width)))){

 					*bools[i] = true;
					break;
				}
				else {
					*bools[i] = false;
				}
			}
		}
	}
}

void GameScene2::pauseMenu(Ref *psender){
	//only open the pause menu if the game is not already paused
	if (!paused){
		paused = true;

		CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic(); //pause the background music

		//create each menu item, set their positions, and add them to an array
		Vector <MenuItem*> menuItems;

		MenuItem* resumeButton = MenuItemImage::create("resumeButton.png", "resumeButton.png", this, menu_selector(GameScene2::resumeGame));
		resumeButton->setPosition(ccp(0, 125));
		menuItems.pushBack(resumeButton);
		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene2::restart));
		restartButton->setPosition(ccp(0, 25));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene2::selectLevel));
		nextButton->setPosition(ccp(0, -75));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene2::menuCloseCallback));
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

void GameScene2::youWinMenu(){
	if (!paused){
		paused = true;

		CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();

		paused = true;

		Vector <MenuItem*> menuItems;

		MenuItem* restartButton = MenuItemImage::create("restartButton.png", "restartButton.png", this, menu_selector(GameScene2::restart));
		restartButton->setPosition(ccp(0, 55));
		menuItems.pushBack(restartButton);
		MenuItem* nextButton = MenuItemImage::create("nextButton.png", "nextButton.png", this, menu_selector(GameScene2::selectLevel));
		nextButton->setPosition(ccp(0, -45));
		menuItems.pushBack(nextButton);
		MenuItem* exitButton = MenuItemImage::create("exitButton.png", "exitButton.png", this, menu_selector(GameScene2::menuCloseCallback));
		exitButton->setPosition(ccp(0, -145));
		menuItems.pushBack(exitButton);

		int totalNotes = level->getNotes("hihat", 4).size() + level->getNotes("cymbal", 2).size() + level->getNotes("snare", 6).size()
			+ level->getNotes("tom1", 8).size() + level->getNotes("tom2", 10).size() + level->getNotes("tom3", 12).size() + level->getNotes("bass", 14).size();

		int percentage = (int)(100 * score / totalNotes);

		if (percentage < 0){
			scoreLabel2->setString(to_string(score) + "/" + to_string(totalNotes) + " = BAD");
		}
		else{
			scoreLabel2->setString(to_string(score) + "/" + to_string(totalNotes) + " = " + to_string(percentage) + "%");
		}

		scoreLabel2->setVisible(true);

		pauseMenu2 = Menu::createWithArray(menuItems);
		pauseMenu2->setPosition(cameraTarget->getPosition());
		pauseMenu2->setGlobalZOrder(10000);
		this->addChild(pauseMenu2, 10000);

		FadeIn *fadeIn = FadeIn::create(.5);
		youWinMenubg->runAction(fadeIn);
		youWinMenubg->setPosition(cameraTarget->getPosition());
	}
}

void GameScene2::resumeGame(Ref *psender){

	pauseMenu2->removeFromParent(); //remove the menu when the game is not pause anymore

	//fade out the background image of the menu
	FadeOut *fadeOut = FadeOut::create(.25);
	pauseMenubg->runAction(fadeOut);

	paused = false; //unpause the game

	CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic(); //resume the background music
}

void GameScene2::restart(Ref *psender){

	//creates a new scene of the same level and restarts it from the beginning
	auto newScene = GameScene2::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene2::selectLevel(Ref *psender){
#if defined(__EMSCRIPTEN__)
    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::getInstance()->stopAllEffects();
#endif

	//replaces current scene with a new scene of the next level
	auto newScene = StartScreen::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f, newScene));
}

void GameScene2::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	//if the game is not paused, it checks to see if keys are pressed
	//for each key pressed, if the corresponding note is there, the sound will be played and the score will go up
	//else if the controls menu is still visible, the sound plays
	//else, the score goes down and an error sound is played

#if defined(__EMSCRIPTEN__)
    if (paused) return;
    updateRhythm();
    if (paused) return;
    const EventKeyboard::KeyCode keys[] = {CYMBAL, HI_HAT, SNARE, TOM1, TOM2, TOM3, SPACEBAR};
    const char* sounds[] = {"cymbal.wav", "hihat.wav", "snare.wav", "tom1.wav", "tom2.wav", "tom3.wav", "bass.wav"};
    for (int lane=0; lane<7; ++lane) {
        if (keyCode != keys[lane]) continue;
        const double seconds = afroman::musicTime(), offset = afroman::calibrationOffset();
        int column = -1;
        const bool hit = rhythm.takeHit(rhythmNotes[lane], seconds, offset, &column);
        const bool practice = !hit && controls->getOpacity()>0;
        if (!hit && !practice) {
            // Only flash an off-time note when it is actually near this press.
            // A distant note must not appear to have been judged early/late.
            double nearest = 0.30;
            for (const auto& note:rhythmNotes[lane]) {
                const double distance = std::abs(seconds-rhythm.noteTime(note.column,offset));
                if (!note.hit && distance < nearest) { nearest=distance; column=note.column; }
            }
        }
        showRhythmFeedback(lane,hit,practice,column);
        if (hit) { ++score; CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(sounds[lane]); }
        else if (practice) CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(sounds[lane]);
        else { --score; CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("error.wav"); }
        scoreLabel1->setString("SCORE: " + to_string(score));
        return;
    }
    return;
#endif
	if (!paused){
		if (keyCode == SPACEBAR){

			if (bass){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("bass.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("bass.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}
		if (keyCode == SNARE){
			if (snare){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("snare.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("snare.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}
		if (keyCode == HI_HAT){
			if (hihat){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("hihat.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("hihat.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}
		if (keyCode == TOM1){
			if (tom1){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("tom1.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("tom1.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}
		if (keyCode == TOM2){
			if (tom2){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("tom2.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("tom2.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}
		if (keyCode == TOM3){

			if (tom3){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("tom3.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("tom3.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}
		if (keyCode == CYMBAL){
			if (cymbal){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("cymbal.wav");
				score++;
			}
			else if (controls->getOpacity() > 0){
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("cymbal.wav");
			}
			else{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("error.wav");
				score--;
			}
		}

		if (std::find(heldKeys.begin(), heldKeys.end(), keyCode) == heldKeys.end()){
			heldKeys.push_back(keyCode);
		}
	}
}

void GameScene2::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	heldKeys.erase(std::remove(heldKeys.begin(), heldKeys.end(), keyCode), heldKeys.end());
}

void GameScene2::menuCloseCallback(Ref* pSender)
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

GameScene2::GameScene2(void)
{
	setKeyboardEnabled(true);
}
GameScene2::~GameScene2(void)
{
}

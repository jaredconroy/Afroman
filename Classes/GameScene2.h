#ifndef  _GAME_SCENE_2_H_
#define  _GAME_SCENE_2_H_

#include "cocos2d.h"
#include "Level.h"
#include "Player.h"
#include <algorithm>
#include <string>
#include <vector>
#if defined(__EMSCRIPTEN__)
#include "RhythmTimeline.h"
#endif

using namespace std;
USING_NS_CC;

class GameScene2 : public cocos2d::Layer
{
public:

	//variables

	Level *level;
	Sprite *bg_sprite;
	Sprite *cameraTarget;

	Follow *camera;

	vector<EventKeyboard::KeyCode> heldKeys;

	LabelTTF* scoreLabel1;
	LabelTTF* scoreLabel2;

	Menu* menu;
	MenuItem* pauseButton;
	Sprite* pauseMenubg;
	Sprite* controls;
	Sprite* youWinMenubg;
	Menu* pauseMenu2;

	Size wsize;
	Point center;

	int score = 0;

	bool cymbal = false;
	bool hihat = false;
	bool snare = false;
	bool tom1 = false;
	bool tom2 = false;
	bool tom3 = false;
	bool bass = false;
	bool paused = false;

#if defined(__EMSCRIPTEN__)
    afroman::RhythmTimeline rhythm;
    bool controlsDismissed = false;
    bool resultsFreePlay = false;
    LabelTTF* practiceLabel = nullptr;
    std::vector<afroman::RhythmNote> rhythmNotes[7];
    void updateRhythm();
    void showRhythmFeedback(int lane, bool hit, bool practice, int column);
#endif

	//methods

	CREATE_FUNC(GameScene2);

	static cocos2d::Scene* createScene();

	virtual bool init();

	void updateScene(float interval);

	void pauseMenu(Ref* pSender);

	void youWinMenu();

	void resumeGame(Ref* pSender);

	void restart(Ref* pSender);

	void selectLevel(Ref* pSender);

	virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	void menuCloseCallback(cocos2d::Ref* pSender);

	GameScene2(void);
	virtual ~GameScene2(void);
};

#endif // _GAME_SCENE_2_H_
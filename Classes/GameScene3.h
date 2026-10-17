#ifndef  _GAME_SCENE_3_H_
#define  _GAME_SCENE_3_H_

#include "cocos2d.h"
#include "Level.h"
#include "Player.h"
#include "GameCountdown.h"
#include <algorithm>
#include <string>
#include <vector>

using namespace std;
USING_NS_CC;

class GameScene3 : public cocos2d::Layer
{
public:

	//variables

	Level * level;
	Player *player;
	Sprite *player_sprite;
	Sprite *point_sprite;
	Sprite *bg_sprite;
	Sprite *cameraTarget;

	Follow *camera;

	Point point;
	Size wsize;
	Point center;

	Menu* menu;
	MenuItem* pauseButton;
	Menu* pauseMenu2;

	Sprite *pauseMenubg;
	Sprite *gameOverMenubg;
	Sprite *youWinMenubg;

	bool paused = false;
	float PLAYER_MAX_VELOCITY = 3.0f;
	int time = 25;
	int timeBySecond = 60;
    afroman::GameCountdown countdown;
    DrawNode* trail = nullptr;

	int minutes;
	int seconds;
	LabelTTF* timeLabel;

	vector<EventKeyboard::KeyCode> heldKeys;

	vector<Rect> tiles;

	//methods

	CREATE_FUNC(GameScene3);

	virtual bool init();

	void updateScene(float interval);

	void updatePlayer(float interval);

	void pauseMenu(Ref* pSender);

	void gameOverMenu();

	void youWinMenu();

	void resumeGame(Ref* pSender);

	void restart(Ref* pSender);

	void selectLevel(Ref* pSender);

	static cocos2d::Scene* createScene();

	virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	void menuCloseCallback(cocos2d::Ref* pSender);

	GameScene3(void);
	virtual ~GameScene3(void);
};

#endif // _GAME_SCENE_3_H_
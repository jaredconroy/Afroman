#ifndef  _GAME_SCENE_H_
#define  _GAME_SCENE_H_

#include "cocos2d.h"
#include "Level.h"
#include "Player.h"
#include "GameCountdown.h"
#include <algorithm>
#include <string>
#include <vector>

using namespace std;
USING_NS_CC;

class GameScene : public cocos2d::Layer
{
public:

	//variables

	Level * level;
	Level * bg;
	Level * fg;

	Player *player;

	Sprite *player_sprite;
	Sprite *cameraTarget;
	Sprite *drumset;
	Sprite *disguise;
	Sprite *pauseMenubg;
	Sprite *gameOverMenubg;
	Sprite *youWinMenubg;

	Animate *walkRight;
	Animate *jumping;
	Animate *climb;
	Animate *dwalkRight;

	LabelTTF* timeLabel;

	MenuItem* pauseButton;

	Menu* menu;
	Menu* pauseMenu2;

	Follow *camera;

	Size wsize;
	Point center;
	Point point;

	bool sewer = false;
	bool slow = false;
	bool ladder = false;
	bool disguised;
	bool paused;
	float PLAYER_MAX_VELOCITY = 8.0f;
	int time = 60;
	int timeBySecond = 60;
    afroman::GameCountdown countdown;
    float ladderCenterX = 0, sewerCenterX = 0, flushBoundaryX = 0;
    bool touchingFlushWall = false;
#if defined(__EMSCRIPTEN__)
    bool bathroomOccupied = false;
    LayerColor* bathroomShade = nullptr;
    DrawNode* bathroomLamp = nullptr;
    LayerColor* bathroomWindowLight = nullptr;
    Sprite* trashBin = nullptr;
    bool touchingTrashBin = false;
    bool touchingCar = false, carLightsOn = false;
    DrawNode* carLights = nullptr;
    void hitTrashBin();
    void updateBathroomLight();
#endif

	int minutes;
	int seconds;
	
	vector<EventKeyboard::KeyCode> heldKeys;

	//methods

	virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

	void updateScene(float interval);

	void updatePlayer(float interval);
	void updatePlayerSprite(float interval);

	void getCollisionTiles(float interval);

	void pauseMenu(Ref* pSender);

	void gameOverMenu();

	void youWinMenu();

	void resumeGame(Ref* pSender);

	void restart(Ref* pSender);

	void selectLevel(Ref* pSender);

	static cocos2d::Scene* createScene();

	virtual bool init();

	void menuCloseCallback(cocos2d::Ref* pSender);
	void setupAnimations();

	CREATE_FUNC(GameScene);

	GameScene(void);
	virtual ~GameScene(void);
};

#endif // _GAME_SCENE_H_
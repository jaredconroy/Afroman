#include "CCApplication.h"
#include "CCGLView.h"
#include "base/CCDirector.h"
#include "2d/CCScene.h"
#include "2d/CCLayer.h"
#include "2d/CCDrawNode.h"
#include "base/CCConsole.h"
#include "2d/CCActionInterval.h"
class SmokeApp : public cocos2d::Application {
 bool applicationDidFinishLaunching() override {
  auto d=cocos2d::Director::getInstance(); auto v=cocos2d::GLView::create("Afroman Web");
  if(!v) return false;
  d->setOpenGLView(v); d->setDisplayStats(false); d->setAnimationInterval(1.0/60);
  auto scene=cocos2d::Scene::create();
  auto background=cocos2d::LayerColor::create(cocos2d::Color4B(22,27,38,255)); scene->addChild(background);
  auto shape=cocos2d::DrawNode::create();
  cocos2d::Vec2 vertices[]={{-100,-80},{100,-80},{100,80},{-100,80}};
  shape->setPosition(525,320);
  shape->drawPolygon(vertices,4,cocos2d::Color4F(0.2,0.9,0.55,1),0,cocos2d::Color4F(0,0,0,0));
  scene->addChild(shape); shape->runAction(cocos2d::RepeatForever::create(cocos2d::RotateBy::create(8,360)));
  d->runWithScene(scene);
  cocos2d::log("AFROMAN_M1: Cocos scene initialized"); return true;
 }
 void applicationDidEnterBackground() override {}
 void applicationWillEnterForeground() override {}
};
int main() { static SmokeApp app; return app.run(); }

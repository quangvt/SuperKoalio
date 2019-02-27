//
//  GameLevelLayer.h
//  SuperKoalio
//
//  Created by Quang Vu on 1/21/19.
//

#ifndef GameLevelLayer_h
#define GameLevelLayer_h

#include "cocos2d.h"
#include "Player.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

struct tileInfo
{
    int gid;
    float x;
    float y;
    Vec2 tilePos;
};

class GameLevelLayer : public Layer
{
    // Fields
    TMXTiledMap *_map;
    TMXLayer *_wallLayer;
    TMXLayer *_hazardLayer;
    Player *_player;
    bool _isGameOver;
    
    // Schedule update handler
    void update(float dt);
    
    // Collision
    Vec2 tileCoordForPosition(Vec2 position);
    Rect tileRectFromTileCoords(Vec2 tileCoords);
    std::vector<tileInfo> getSurroundingTilesAtPosition(Vec2 position, TMXLayer* layer);
    void handleHazardCollisions(Player* player);
    void checkForAndResolveCollisions(Player *player);
    
    // Camera on screen
    void setViewpointCenter(Vec2 position);
    
    // Menu
    void gameOver(bool playerDidWin);
    
    // Menu Callback
    void replayButtonCallback(Ref* pSender);
    
    // Helper
    void checkForWin();
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    // implement the "static create()" method manually
    CREATE_FUNC(GameLevelLayer);
    
    // Touch
    virtual void onTouchesBegan(const std::vector<Touch*>& touches, Event *event);
    virtual void onTouchesMoved(const std::vector<Touch*>& touches, Event *event);
    virtual void onTouchesEnded(const std::vector<Touch*>& touches, Event *event);
};

#endif /* GameLevelLayer_h */

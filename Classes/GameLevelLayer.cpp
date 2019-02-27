//
//  GameLevelLayer.cpp
//  SuperKoalio
//
//  Created by Quang Vu on 1/21/19.
//

#include "GameLevelLayer.h"

USING_NS_CC;

Scene* GameLevelLayer::createScene()
{
    Scene* scene = Scene::create();
    Layer* layer = GameLevelLayer::create();
    scene->addChild(layer);
    return scene;
}

bool GameLevelLayer::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    // set background color
    LayerColor *blueSky = LayerColor::create(Color4B(100, 100, 250, 255));
    this->addChild(blueSky);
    
    // Create map
    _map = TMXTiledMap::create("level1.tmx");
    this->addChild(_map, 0);
    
    // Init player
    _player = Player::create("koalio_stand.png");
    _player->setPosition(Vec2(100, 50));
    _map->addChild(_player, 15);
    
    // Init data for layer
    _wallLayer = _map->getLayer("walls");
    _hazardLayer = _map->getLayer("hazards");
    
    // Touch
    auto touchListener = EventListenerTouchAllAtOnce::create();
    touchListener->onTouchesBegan = CC_CALLBACK_2(GameLevelLayer::onTouchesBegan, this);
    touchListener->onTouchesMoved = CC_CALLBACK_2(GameLevelLayer::onTouchesMoved, this);
    touchListener->onTouchesEnded = CC_CALLBACK_2(GameLevelLayer::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    // Autio
    CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic("level1.mp3");
    
    // Run update
    this->scheduleUpdate();
    
    return true;
}

/**
 * Update handler function
 */
void GameLevelLayer::update(float delta) {
    if(_isGameOver)
    {
        return;
    }
    _player->update(delta);
    this->handleHazardCollisions(_player);
    this->checkForWin();
    this->checkForAndResolveCollisions(_player);
    this->setViewpointCenter(_player->getPosition());
}

// Touches
void GameLevelLayer::onTouchesBegan(const std::vector<Touch*>& touches, Event *event)
{
    for (int i=0;i<touches.size();++i)
    {
        Vec2 touchLocation = convertTouchToNodeSpace(touches[i]);
        if (touchLocation.x > Director::getInstance()->getVisibleSize().width/2)
        {
            // Right side of screen: Jump
            _player->setIsJumpingFlag(true);
        }
        else
        {
            // Left side of screen: Move
            _player->setIsMovingFlag(true);
        }
    }
}

void GameLevelLayer::onTouchesMoved(const std::vector<Touch*>& touches, Event *unused_event)
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    for (int i=0;i<touches.size();++i)
    {
        Vec2 touchLocation = convertTouchToNodeSpace(touches[i]);
        //get previous touch and convert it to node space
        Vec2 previousTouchLocation = touches[i]->getPreviousLocationInView();
        
        previousTouchLocation = Vec2(previousTouchLocation.x,
                                     visibleSize.height - previousTouchLocation.y);
        if (touchLocation.x > visibleSize.width/2 &&
            previousTouchLocation.x <= visibleSize.width/2)
        {
            _player->setIsMovingFlag(false);
            _player->setIsJumpingFlag(true);
        }
        else if (previousTouchLocation.x > visibleSize.width/2 &&
                 touchLocation.x <= visibleSize.width/2)
        {
            _player->setIsMovingFlag(true);
            _player->setIsJumpingFlag(false);
        }
    }
}

void GameLevelLayer::onTouchesEnded(const std::vector<Touch*>& touches, Event *unused_event)
{
    for (int i=0;i<touches.size();++i)
    {
        Vec2 touchLocation = convertTouchToNodeSpace(touches[i]);
        Size visibleSize = Director::getInstance()->getVisibleSize();
        if (touchLocation.x < visibleSize.width/2)
        {
            _player->setIsMovingFlag(false);
        }
        else
        {
            _player->setIsJumpingFlag(false);
        }
    }
}

/**
 * Tile's coordinates
 * x: position.x / tileSize.width
 * y: (Map's size - position.y) / tileSize.height
 *
 * Revert y value because: Cocos has the origin from bottom left, but
 *   tile map has the orgin from top left.
 */
Point GameLevelLayer::tileCoordForPosition(Point position)
{
    auto tileSize = _map->getTileSize();
    auto mapSize = _map->getMapSize();
    float x = floor(position.x / tileSize.width);
    float levelHeightInPixels = mapSize.height * tileSize.height;
    float y = floor((levelHeightInPixels - position.y) / tileSize.height);
    return Point(x, y);
}

/**
 * Get the Tile's Rect in Cocos from a tileCoordinates
 *   x: tileCoordinates.x * tileSize.width
 *   y: Map's height in pixel - (tileCoordinates.y + 1)*tileSize.height;
 * Note:
 *   tileCoordinate.y + 1: because tilemap is a base 0 indexing.
 */
Rect GameLevelLayer::tileRectFromTileCoords(Vec2 tileCoords)
{
    auto tileSize = _map->getTileSize();
    auto mapSize = _map->getMapSize();
    float levelHeightInPixels = mapSize.height * tileSize.height;
    Point origin = Point(tileCoords.x * tileSize.width, levelHeightInPixels - ((tileCoords.y + 1) * tileSize.height));
    return Rect(origin.x, origin.y, tileSize.width, tileSize.height);
}

/**
 * Get list of 8 surrounding tiles of player's position.
 */
std::vector<tileInfo> GameLevelLayer::getSurroundingTilesAtPosition(Vec2 position, TMXLayer *layer)
{
    // Player's position in tilemap
    Point playerPos = tileCoordForPosition(position);
    
    // List of player's surrounding tiles
    std::vector<tileInfo> gids;
    
    // Iterate over 9 tiles (8 surrounding tiles of player and player tile)
    for (int i = 0; i < 9; i++)
    {
        // Calculate the surrounding tile position in tilemap
        int col = i % 3;
        int row = (int)(i / 3);
        Vec2 surTilePos = Vec2(playerPos.x + (col - 1), playerPos.y + (row - 1));
        
        // Fall in a hole (over ground) => game over
        if (surTilePos.y > (_map->getMapSize().height - 1))
        {
            gameOver(false);
            return gids;
        }
        
        // Collided with layer's tile (eg: wall layer, hazard layer, ...)
        int tileGid = layer->getTileGIDAt(surTilePos);
        // Get world position of the surrounding tile
        Rect tileRect = tileRectFromTileCoords(surTilePos);
        
        // Fill all info to the surrounding tile
        tileInfo tileItem;
        tileItem.gid = tileGid;
        tileItem.x = tileRect.origin.x;
        tileItem.y = tileRect.origin.y;
        tileItem.tilePos = surTilePos;
        
        gids.push_back(tileItem);
    }
    
    // Priority of collided test: Bottom, Top, Left, Right
    gids.erase(gids.begin() + 4);
    gids.insert(gids.begin() + 6, gids[2]);
    gids.erase(gids.begin() + 2);
    
    tileInfo temp;
    temp = gids[6];
    gids[6] = gids[4];
    gids[4] = temp;
    
    temp = gids[0];
    gids[0] = gids[4];
    gids[4] = temp;
 
    // print test
//    for (int i=0;i<gids.size();++i)
//    {
//        printf("i: %d\n", i);
//        printf("gid: %d\n", gids[i].gid);
//        printf("tilePos.x: %f\n", gids[i].tilePos.x);
//        printf("tilePos.y: %f\n", gids[i].tilePos.y);
//        printf("x: %f\n", gids[i].x);
//        printf("y: %f\n\n", gids[i].y);
//    }
    
    return gids;
}

/**
 *
 */
void GameLevelLayer::handleHazardCollisions(Player* player)
{
    std::vector<tileInfo> surTiles =
        getSurroundingTilesAtPosition(player->getPosition(), _hazardLayer);
    
    for (int i = 0; i < surTiles.size(); i++)
    {
        if (surTiles[i].gid)
        {
            Rect collTileRect = Rect(surTiles[i].x,
                                     surTiles[i].y,
                                     _map->getTileSize().width,
                                     _map->getTileSize().height);
            Rect playerRect = player->getCollisionBoundingBox();
            if (playerRect.intersectsRect(collTileRect))
            {
                gameOver(false);
            }
        }
    }
}

/**
 * Check & Resolve collisions with the surrouding tiles of player
 *
 */
void GameLevelLayer::checkForAndResolveCollisions(Player *player)
{
    std::vector<tileInfo> surTiles = this->getSurroundingTilesAtPosition(player->getPosition(), _wallLayer);

    // Fall in a hole
    if (_isGameOver)
    {
        return;
    }
    
    player->setOnGroundFlag(false);
    
    for (int i = 0; i < surTiles.size(); ++i)
    {
        Rect playerRect = player->getCollisionBoundingBox();
        
        int gid = surTiles[i].gid;
        
        // Collided
        if (gid)
        {
            Rect collTileRect = Rect(surTiles[i].x,
                                 surTiles[i].y,
                                 _map->getTileSize().width,
                                 _map->getTileSize().height);
            if (playerRect.intersectsRect(collTileRect))
            {
                // This is replicating CGRectIntersection
                Rect intersection = Rect(std::max(playerRect.getMinX(),collTileRect.getMinX()),
                                         std::max(playerRect.getMinY(),collTileRect.getMinY()),
                                         0,
                                         0);
                intersection.size.width = std::min(playerRect.getMaxX(),collTileRect.getMaxX())
                    - intersection.getMinX();
                intersection.size.height = std::min(playerRect.getMaxY(),collTileRect.getMaxY())
                    - intersection.getMinY();
                
                switch (i)
                {
                    case 0: // Bottom: directly below Koala
                        player->setDesiredPosition(Vec2(player->getDesiredPosition().x,
                                                        player->getDesiredPosition().y + intersection.size.height));
                        player->setVelocity(Vec2(player->getVelocity().x, 0.0));
                        player->setOnGroundFlag(true);
                        break;
                    case 1: // Top: directly above Koala
                        player->setDesiredPosition(Vec2(player->getDesiredPosition().x,
                                                        player->getDesiredPosition().y - intersection.size.height));
                        player->setVelocity(Vec2(player->getVelocity().x, 0.0));
                        break;
                    case 2: // Left
                        player->setDesiredPosition(Vec2(player->getDesiredPosition().x + intersection.size.width,
                                                        player->getDesiredPosition().y));
                        break;
                    case 3: // Right
                        player->setDesiredPosition(Vec2(player->getDesiredPosition().x - intersection.size.width,
                                                        player->getDesiredPosition().y));
                        break;
                    default: // Other: 4 (Top Left), 5 (Top Right), 6 (Bottom Left), 7 (Bottom Right)
                        // Vertical Collision
                        if (intersection.size.width > intersection.size.height)
                        {
                            // tile is diagonal, but resolving collision vertically
                            player->setVelocity(Vec2(player->getVelocity().x, 0.0));
                            
                            float resolutionHeight;
                            if (i == 6 || i == 7)  // 6, 7: Bottom Left, Bottom Right tiles
                            {
                                resolutionHeight = intersection.size.height;
                                player->setOnGroundFlag(true);
                            }
                            else // 4, 5: Top Left, Top Right tiles
                            {
                                resolutionHeight = -intersection.size.height;
                            }
                            player->setDesiredPosition(Vec2(player->getDesiredPosition().x,
                                                            // player->getDesiredPosition().y + intersection.size.height ));
                                                            player->getDesiredPosition().y + resolutionHeight ));
                        }
                        else // Horizontal Collision
                        {
                            //tile is diagonal, but resolving horizontally
                            float resolutionWidth;
                            
                            if (i == 4 || i == 6) // 4: Top Left tile, 6: Top Right tile
                            {
                                resolutionWidth = intersection.size.width;
                            }
                            else // 5: Top Right, 7: Bottom Right
                            {
                                resolutionWidth = -intersection.size.width;
                            }
                            player->setDesiredPosition(Vec2(player->getDesiredPosition().x,
                                                            player->getDesiredPosition().y + resolutionWidth));
                        }
                        break;
                }
            }
        }
    }
    player->setPosition(player->getDesiredPosition());
}

/**
 *
 */
void GameLevelLayer::setViewpointCenter(Vec2 position)
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    
    int x = MAX(position.x, visibleSize.width / 2);
    int y = MAX(position.y, visibleSize.height / 2);
    x = MIN(x, (_map->getMapSize().width * _map->getTileSize().width)
            - visibleSize.width / 2);
    y = MIN(y, (_map->getMapSize().height * _map->getTileSize().height)
            - visibleSize.height/2);
    Vec2 actualPosition = Vec2(x, y);
    
    Vec2 centerOfView = Vec2(visibleSize.width/2, visibleSize.height/2);
    Vec2 viewPoint = centerOfView - actualPosition;
    _map->setPosition(viewPoint);
}


/**
 *
 */
void GameLevelLayer::gameOver(bool playerDidWin)
{
    _isGameOver = true;
    std::string gameText;
    
    if (playerDidWin)
    {
        gameText = "You Won!";
    }
    else
    {
        gameText = "You have died!";
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("hurt.wav");
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Menu* pMenu = Menu::create();
    pMenu->setPosition(Vec2(visibleSize.width/2, -50));
    
    MenuItemFont *diedLabel = MenuItemFont::create(gameText);
    diedLabel->setFontName("Marker Felt");
    diedLabel->setFontSize(24);
    diedLabel->setPosition(Vec2(0, 50));
    pMenu->addChild(diedLabel);
    
    MoveBy *slideIn = MoveBy::create(1.0, Vec2(0,200));
    
    MenuItemImage *replay = MenuItemImage::create("replay.png", "replay.png", "replay.png");
    replay->setPosition(Point::ZERO);
    replay->setCallback(CC_CALLBACK_1(GameLevelLayer::replayButtonCallback, this));
    pMenu->addChild(replay);
    
    this->addChild(pMenu, 1);

    pMenu->runAction(slideIn);
}

void GameLevelLayer::replayButtonCallback(Ref* pSender)
{
    Director::getInstance()->replaceScene(GameLevelLayer::createScene());
}

void GameLevelLayer::checkForWin()
{
    if (_player->getPosition().x > 3130.0)
    {
        gameOver(true);
    }
}

//
//  Player.h
//  SuperKoalio
//
//  Created by Quang Vu on 1/21/19.
//

#ifndef Player_h
#define Player_h
#include "SimpleAudioEngine.h"

USING_NS_CC;

class Player : public Sprite
{
    // Default Data/Function Members of C++ class is private
    // Note: struct is versus: default is public
    Vec2 _velocity;
    Vec2 _desiredPosition;
    bool _isOnGround;
    bool _isJumping;
    bool _isMoving;
public:
    // Accessors
    void setDesiredPosition(const Vec2&);
    Vec2 getDesiredPosition();
    void setVelocity(const Vec2&);
    Vec2 getVelocity();
    void setOnGroundFlag(const bool);
    bool getOnGroundFlag();
    void setIsMovingFlag(const bool);
    bool getIsMovingFlag();
    void setIsJumpingFlag(const bool);
    bool getIsJumpingFlag();
    
    // Create Sprite
    bool initWithFile(const std::string &filename) override;
    static Player* create(const std::string &filename);
    
    // Scheduled methods
    void update(const float dt) override;
    
    // Get bounding box of player
    Rect getCollisionBoundingBox();
};

#endif /* Player_h */

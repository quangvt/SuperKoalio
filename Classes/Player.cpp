//
//  Player.cpp
//  SuperKoalio
//
//  Created by Quang Vu on 1/21/19.
//
#include "Player.h"

USING_NS_CC;

/**
 *
 */
bool Player::initWithFile(const std::string& filename)
{
    if (!Sprite::initWithFile(filename))
    {
        return false;
    }
    _isJumping = false;
    _isMoving = false;
    _isOnGround = false;
    
    _desiredPosition = Vec2::ZERO;
    _velocity = Vec2::ZERO;
    return true;
}

// Accessors
void Player::setDesiredPosition(const Vec2& value)
{
    _desiredPosition = value;
}

Vec2 Player::getDesiredPosition()
{
    return _desiredPosition;
}

void Player::setVelocity(const Vec2& value)
{
    _velocity = value;
}

Vec2 Player::getVelocity()
{
    return _velocity;
}

void Player::setOnGroundFlag(const bool value)
{
    _isOnGround = value;
}

bool Player::getOnGroundFlag()
{
    return _isOnGround;
}

void Player::setIsMovingFlag(const bool value)
{
    _isMoving = value;
}

bool Player::getIsMovingFlag()
{
    return _isMoving;
}

void Player::setIsJumpingFlag(const bool value)
{
    _isJumping = value;
}

bool Player::getIsJumpingFlag()
{
    return _isJumping;
}

/**
 *
 */
Player* Player::create(const std::string &filename)
{
    Player* sprite = new Player();
    // TODO: Review these code lines
    if(sprite && sprite->initWithFile(filename))
    {
        // TODO: ???
        sprite->autorelease();
        
        return sprite;
    }
    return NULL;
}

/**
 *
 */
void Player::update(float delta)
{
    // Declare forces
    // Gravity force
    Vec2 gravity = Vec2(0.0, -450.0);
    Vec2 gravityStep = delta * gravity;
    // Friction force ratio
    float frictionRatio = 0.90f;
    // Jump force
    Vec2 jumpForce = Vec2(0.0, 310.0);
    float jumpCutoff = 150.0;
    // Moving forward force
    Vec2 forwardMove = Vec2(800.0, 0.0);
    Vec2 forwardStep = forwardMove * delta;
    
    // Apply forces
    // Apply gravity force
    _velocity += gravityStep;
    // Apply friction force
    _velocity = Vec2(_velocity.x * frictionRatio, _velocity.y);
    // Apply jump force
    if (_isJumping && _isOnGround)
    {
        _velocity += jumpForce;
        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("jump.wav");
    }
    else if (!_isJumping && _velocity.y > jumpCutoff)
    {
        _velocity = Vec2(_velocity.x, jumpCutoff);
    }
    // Apply move forward force
    if (_isMoving) {
        _velocity += forwardStep;
    }
    
    // Fasten the velocity in a range
    Vec2 minMovement = Vec2(0.0, -450.0);
    Vec2 maxMovement = Vec2(120.0, 250.0);
    _velocity = _velocity.getClampPoint(minMovement, maxMovement);
    
    // Step for movement by velocity
    Vec2 stepVelocity = _velocity * delta;
    Vec2 movementVector = getPosition() + stepVelocity;
    
    // Update position
    this->setDesiredPosition(movementVector);
    this->setPosition(getDesiredPosition());
}

Rect Player::getCollisionBoundingBox()
{
    // What are 3, 6?
    // CGRectInset shrinks a CGRect by the number of pixels specified
    // in the second and third arguments. So in this case, the width of
    // your collision bounding box will be six pixels smaller — three
    // on each side — than the bounding box based on the image file
    // you’re using.
    
    //Inset
    Rect collisionBox = Rect(getBoundingBox().origin.x + 3,
                             getBoundingBox().origin.y,
                             getBoundingBox().size.width - 6,
                             getBoundingBox().size.height);
    
    Vec2 diff = _desiredPosition - getPosition();
    
    //Offset
    Rect returnBoundingBox = Rect(collisionBox.origin.x + diff.x,
                                  collisionBox.origin.y + diff.y,
                                  collisionBox.size.width,
                                  collisionBox.size.height);
    
    return returnBoundingBox;
}

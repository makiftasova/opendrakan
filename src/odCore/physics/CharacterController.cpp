/*
 * CharacterController.cpp
 *
 *  Created on: May 23, 2018
 *      Author: zal
 */

#include <odCore/physics/CharacterController.h>

#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>

#include <odCore/LevelObject.h>
#include <odCore/Level.h>

#include <odCore/physics/PhysicsSystem.h>
#include <odCore/physics/Handles.h>

namespace odPhysics
{

    // all lengths are in world units. a character is a bit less than half a unit tall
    static const float STEP_HEIGHT = 0.09f;         // highest step that is climbed without any jumping
static const float GROUND_PROBE_HEIGHT = 0.25f; // height above the feet from which we look for ground. lets us recover if we ended up (slightly) below the ground
    static const float GRAVITY = 2.5f;              // acceleration while falling
    static const float MAX_FALL_SPEED = 6.0f;
    static const float MIN_GROUND_NORMAL_Y = 0.5f;  // surfaces steeper than this can't be walked on
    static const float MAX_WALL_NORMAL_Y = 0.7f;    // surfaces flatter than this don't block horizontal movement
    static const float SKIN = 0.005f;               // distance to keep from walls
    static const float SWIM_ENTER_DEPTH = 0.30f;    // water depth (over the ground) at which the character starts to swim
    static const float SWIM_EXIT_DEPTH = 0.22f;     // water depth below which it stops swimming. lower than the entry depth, so it doesn't flicker
    static const float SWIM_SUBMERSION = 0.30f;     // how far the feet are below the surface when swimming. the head stays out of the water
    static const float SWIM_BED_CLEARANCE = 0.03f;  // how close to the ground a diving character can get
    static const float SWIM_LIFT_SPEED = 1.5f;      // how fast the character moves to the swimming height
    static const PhysicsTypeMasks::Mask SOLIDS = PhysicsTypeMasks::Layer | PhysicsTypeMasks::LevelObject;


	CharacterController::CharacterController(PhysicsSystem &physicsSystem, std::shared_ptr<ObjectHandle> handle, od::LevelObject &charObject, float radius, float height)
	: mPhysicsSystem(physicsSystem)
    , mCharObject(charObject)
	, mObjectHandle(handle)
	, mRadius(radius)
	, mHeight(height)
	, mFootOffset(0.0f)
	, mPendingMovement(0.0f)
	, mIsOnGround(false)
	, mIsSwimming(false)
	, mIsUnderwater(false)
	, mForceSurface(false)
	, mSwimVerticalSpeed(0.0f)
	, mFallSpeed(0.0f)
	{
	    // the object's origin is not necessarily at the character's feet. find out how far above them it is
	    auto bounds = mCharObject.getBoundingBox();
	    float footOffset = mCharObject.getPosition().y - bounds.min().y;
	    if(footOffset > 0.0f && footOffset < 10.0f)
	    {
	        mFootOffset = footOffset;
	        mHeight = std::max(mHeight, bounds.max().y - bounds.min().y);
	    }
	}

    void CharacterController::moveRelative(const glm::vec3 &relTranslation, float relTime)
    {
        // motion is reported in the character's own frame. it only gets applied in update(), where we can look at the world
        mPendingMovement += mCharObject.getRotation()*relTranslation;
    }

	void CharacterController::update(float relTime)
	{
	    glm::vec3 position = mCharObject.getPosition();
	    glm::vec3 newPosition = position;

	    // vertical motion is up to us (gravity, steps), so ignore what the animation wants
	    glm::vec3 movement(mPendingMovement.x, 0.0f, mPendingMovement.z);
	    mPendingMovement = glm::vec3(0.0f);

	    if(glm::length(movement) > 1e-6f)
	    {
	        newPosition = _moveHorizontally(newPosition, movement);
	    }

	    if(!_updateSwimming(newPosition, relTime))
	    {
	        newPosition = _followGround(newPosition, relTime);
	    }

	    if(newPosition != position)
	    {
	        mCharObject.setPosition(newPosition);
	    }
	}

	float CharacterController::_findFreeDistance(const glm::vec3 &position, float feetY, const glm::vec3 &direction, float distance, glm::vec3 &hitNormal, bool &hitSomething)
	{
	    // obstacles that are lower than the highest step we can climb don't block us (the ground following will lift us onto them).
	    //  everything else is tested at a few heights along the character's body
	    const float heights[] = { feetY + STEP_HEIGHT + 0.01f, feetY + 0.5f*mHeight, feetY + 0.85f*mHeight };

	    float freeDistance = distance;
	    hitSomething = false;

	    for(float height : heights)
	    {
	        glm::vec3 from(position.x, height, position.z);
	        glm::vec3 to = from + direction*(distance + mRadius);

	        RayTestResult result;
	        if(mPhysicsSystem.rayTestClosest(from, to, SOLIDS, mObjectHandle, result) && std::fabs(result.hitNormal.y) < MAX_WALL_NORMAL_Y)
	        {
	            float hitDistance = result.hitFraction*(distance + mRadius) - mRadius - SKIN;
	            if(hitDistance < freeDistance)
	            {
	                freeDistance = std::max(0.0f, hitDistance);
	                hitNormal = result.hitNormal;
	                hitSomething = true;
	            }
	        }
	    }

	    return freeDistance;
	}

	glm::vec3 CharacterController::_moveHorizontally(const glm::vec3 &position, const glm::vec3 &movement)
	{
	    float feetY = position.y - mFootOffset;

	    float distance = glm::length(movement);
	    glm::vec3 direction = movement / distance;

	    glm::vec3 hitNormal(0.0f);
	    bool blocked = false;
	    float freeDistance = _findFreeDistance(position, feetY, direction, distance, hitNormal, blocked);

	    glm::vec3 newPosition = position + direction*freeDistance;

	    if(blocked)
	    {
	        // slide along the wall for the rest of the movement
	        glm::vec3 remaining = direction*(distance - freeDistance);
	        remaining -= hitNormal*glm::dot(remaining, hitNormal);
	        remaining.y = 0.0f;

	        float slideDistance = glm::length(remaining);
	        if(slideDistance > 1e-6f)
	        {
	            glm::vec3 slideDirection = remaining / slideDistance;

	            glm::vec3 slideNormal(0.0f);
	            bool slideBlocked = false;
	            float slideFree = _findFreeDistance(newPosition, feetY, slideDirection, slideDistance, slideNormal, slideBlocked);

	            newPosition += slideDirection*slideFree;
	        }
	    }

	    return newPosition;
	}

	bool CharacterController::_updateSwimming(glm::vec3 &position, float relTime)
	{
	    float feetY = position.y - mFootOffset;

	    auto waterY = mCharObject.getLevel().getWaterSurfaceHeightAt(glm::vec2(position.x, position.z), feetY - 0.02f);
	    if(!waterY.has_value())
	    {
	        mIsSwimming = false;
	        mIsUnderwater = false;
	        return false;
	    }

	    // find the ground below the water to know how deep the water is
	    glm::vec3 from(position.x, std::min(feetY + GROUND_PROBE_HEIGHT, waterY.value()), position.z);
	    glm::vec3 to(position.x, waterY.value() - 20.0f, position.z);
	    RayTestResult result;
	    float groundY = (mPhysicsSystem.rayTestClosest(from, to, SOLIDS, mObjectHandle, result) && result.hitNormal.y > MIN_GROUND_NORMAL_Y) ? result.hitPoint.y : (waterY.value() - 20.0f);
	    float depth = waterY.value() - groundY;

	    if(!mIsSwimming)
	    {
	        if(depth >= SWIM_ENTER_DEPTH && feetY <= waterY.value() - 0.10f)
	        {
	            mIsSwimming = true;
	            mIsOnGround = false;
	            mFallSpeed = 0.0f;
	        }

	    }else if(depth < SWIM_EXIT_DEPTH)
	    {
	        mIsSwimming = false;
	    }

	    if(!mIsSwimming)
	    {
	        mIsUnderwater = false;
	        return false;
	    }

	    // the feet stay between the ground and the surface level at which the character floats with its head out of the water
	    float highestFeetY = waterY.value() - SWIM_SUBMERSION;
	    float lowestFeetY = std::min(groundY + SWIM_BED_CLEARANCE, highestFeetY);

	    float verticalSpeed = mForceSurface ? SWIM_LIFT_SPEED : mSwimVerticalSpeed;
	    float newFeetY = feetY + verticalSpeed*relTime;

	    // if we are outside the allowed range (e.g. we just fell in), move back into it smoothly
	    float maxCorrection = SWIM_LIFT_SPEED*relTime;
	    if(feetY > highestFeetY)
	    {
	        newFeetY = std::max(highestFeetY, std::min(newFeetY, feetY - maxCorrection));

	    }else if(feetY < lowestFeetY)
	    {
	        newFeetY = std::min(lowestFeetY, std::max(newFeetY, feetY + maxCorrection));

	    }else
	    {
	        newFeetY = glm::clamp(newFeetY, lowestFeetY, highestFeetY);
	    }

	    position.y = newFeetY + mFootOffset;
	    mIsUnderwater = (newFeetY + 0.9f*mHeight) < waterY.value();

	    return true;
	}

	glm::vec3 CharacterController::_followGround(const glm::vec3 &position, float relTime)
	{
	    glm::vec3 newPosition = position;
	    float feetY = position.y - mFootOffset;

	    // how far below the feet we look for ground. when we are standing on something, we follow it down to a certain degree.
	    //  when we are falling, we have to look as far as we are going to fall in this step
	    float lookDown = mIsOnGround ? STEP_HEIGHT : (mFallSpeed + GRAVITY*relTime)*relTime;

	    glm::vec3 from(position.x, feetY + GROUND_PROBE_HEIGHT, position.z);
	    glm::vec3 to(position.x, feetY - lookDown, position.z);

	    RayTestResult result;
	    bool foundGround = mPhysicsSystem.rayTestClosest(from, to, SOLIDS, mObjectHandle, result) && result.hitNormal.y > MIN_GROUND_NORMAL_Y;

	    if(foundGround)
	    {
	        newPosition.y = result.hitPoint.y + mFootOffset;
	        mIsOnGround = true;
	        mFallSpeed = 0.0f;

	    }else
	    {
	        mIsOnGround = false;
	        mFallSpeed = std::min(mFallSpeed + GRAVITY*relTime, MAX_FALL_SPEED);
	        newPosition.y -= mFallSpeed*relTime;
	    }

	    return newPosition;
	}

}

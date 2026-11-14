/*
 * CharacterController.h
 *
 *  Created on: May 23, 2018
 *      Author: zal
 */

#ifndef INCLUDE_PHYSICS_CHARACTERCONTROLLER_H_
#define INCLUDE_PHYSICS_CHARACTERCONTROLLER_H_

#include <memory>

#include <glm/vec3.hpp>

#include <odCore/anim/BoneAccumulator.h>

namespace od
{
    class LevelObject;
}

namespace odPhysics
{

	class PhysicsSystem;
	class ObjectHandle;

	/**
	 * @brief Moves a character through the world: applies the motion reported by animations, slides along walls, follows the ground
	 *        (including steps up and down) and lets the character fall if there is no ground.
	 *
	 * This is a simple implementation based on ray tests. It treats the character as a vertical line with some radius.
	 */
	class CharacterController : public odAnim::BoneAccumulator
	{
	public:

		/**
		 * @param radius    Horizontal distance from the character's center at which obstacles start to block it.
		 * @param height    Height of the character. Obstacles above the character's head are ignored.
		 */
		CharacterController(PhysicsSystem &physicsSystem, std::shared_ptr<ObjectHandle> handle, od::LevelObject &charObject, float radius, float height);

		// implement odAnim::BoneAccumulator
        virtual void moveRelative(const glm::vec3 &relTranslation, float relTime) override;

		void update(float relTime);

		inline bool isOnGround() const { return mIsOnGround; }

		/**
		 * @brief True while the character swims. It starts to swim when the water is deep enough and then floats at the surface.
		 */
		inline bool isSwimming() const { return mIsSwimming; }

		/**
		 * @brief True while the character's head is below the water surface.
		 */
		inline bool isUnderwater() const { return mIsUnderwater; }

		/**
		 * @brief Sets how fast the swimming character moves up (positive) or down (negative), in world units per second.
		 * It can't leave the water at the top or dive through the ground.
		 */
		inline void setSwimVerticalSpeed(float speed) { mSwimVerticalSpeed = speed; }

		/**
		 * @brief While enabled, a swimming character rises to the surface (e.g. when it ran out of air).
		 */
		inline void setForceSurface(bool force) { mForceSurface = force; }


	private:

		glm::vec3 _moveHorizontally(const glm::vec3 &position, const glm::vec3 &movement);
		glm::vec3 _followGround(const glm::vec3 &position, float relTime);

		/**
		 * @brief Decides whether we swim, and if so, keeps the character floating at the surface.
		 * @return True if the character is swimming. Then, the position has been updated and no ground following should happen.
		 */
		bool _updateSwimming(glm::vec3 &position, float relTime);

		/**
		 * @brief Finds how far the character can move in the given direction (a unit vector) before hitting a wall.
		 *
		 * @param feetY       Height of the character's feet.
		 * @param hitNormal   Receives the normal of the surface hit, if any.
		 * @return The distance that can be moved. This is at most the passed distance.
		 */
		float _findFreeDistance(const glm::vec3 &position, float feetY, const glm::vec3 &direction, float distance, glm::vec3 &hitNormal, bool &hitSomething);

        PhysicsSystem &mPhysicsSystem;
		od::LevelObject &mCharObject;
		std::shared_ptr<ObjectHandle> mObjectHandle;

		float mRadius;
		float mHeight;
		float mFootOffset; // distance of the character object's origin above the feet

		glm::vec3 mPendingMovement;

		bool mIsOnGround;
		bool mIsSwimming;
		bool mIsUnderwater;
		bool mForceSurface;
		float mSwimVerticalSpeed;
		float mFallSpeed;
	};

}

#endif /* INCLUDE_PHYSICS_CHARACTERCONTROLLER_H_ */

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


	private:

		glm::vec3 _moveHorizontally(const glm::vec3 &position, const glm::vec3 &movement);
		glm::vec3 _followGround(const glm::vec3 &position, float relTime);

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
		float mFallSpeed;
	};

}

#endif /* INCLUDE_PHYSICS_CHARACTERCONTROLLER_H_ */

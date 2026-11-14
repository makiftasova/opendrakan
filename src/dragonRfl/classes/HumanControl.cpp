/*
 * HumanControl.cpp
 *
 *  Created on: 2 Apr 2018
 *      Author: zal
 */

#include <dragonRfl/classes/HumanControl.h>

#include <cmath>
#include <algorithm>


#include <dragonRfl/RflDragon.h>

#include <vector>

#include <glm/gtc/constants.hpp>

#include <odCore/LevelObject.h>
#include <odCore/Level.h>
#include <odCore/Client.h>
#include <odCore/Server.h>

#include <odCore/anim/Skeleton.h>
#include <odCore/anim/SkeletonAnimationPlayer.h>

#include <odCore/rfl/Rfl.h>
#include <odCore/rfl/PrefetchProbe.h>

#include <odCore/input/InputManager.h>
#include <odCore/input/Keys.h>

#include <odCore/audio/SoundSystem.h>

#include <odCore/physics/PhysicsSystem.h>

#include <odCore/render/Renderer.h>

#include <dragonRfl/classes/TrackingCamera.h>

namespace dragonRfl
{

    static const float TURN_ANIM_THRESHOLD = glm::half_pi<float>(); // angular yaw speed at which turn animation is triggered (in rad/sec)


    HumanControl_Sv::HumanControl_Sv(odNet::ClientId clientId)
    : mClientId(clientId)
    , mYaw(0)
	, mPitch(0)
    , mState(State::Idling)
    , mLastUpdatedYaw(0)
    {
    }

    // looking up or down by less than this (in radians) doesn't dive or surface
    static const float SWIM_PITCH_DEADZONE = 0.12f;

    HumanControl_Sv::~HumanControl_Sv()
    {
    }

    void HumanControl_Sv::onLoaded()
    {
        // prefetch referenced assets
        odRfl::PrefetchProbe probe(getLevelObject().getClass()->getDependencyTable());
        mFields.probeFields(probe);

        // configure controls FIXME: these handlers are not memory safe because actions are not uniquely owned!
        auto actionHandler = std::bind(&HumanControl_Sv::_handleAction, this, std::placeholders::_1, std::placeholders::_2);
        auto analogActionHandler = std::bind(&HumanControl_Sv::_handleAnalogAction, this, std::placeholders::_1, std::placeholders::_2);

        auto &inputManager = getServer().getInputManagerForClient(mClientId);

        auto &forwardAction = inputManager.getAction(Action::Forward);
        forwardAction.setRepeatable(false);
        forwardAction.addCallback(actionHandler);

        auto &backwardAction = inputManager.getAction(Action::Backward);
        backwardAction.setRepeatable(false);
        backwardAction.addCallback(actionHandler);

        auto &attackAction = inputManager.getAction(Action::Attack_Primary);
        attackAction.setRepeatable(false);
        attackAction.addCallback(actionHandler);
        attackAction.setIgnoreUpEvents(true);

        auto &lookAction = inputManager.getAnalogAction(Action::Look);
        lookAction.addCallback(analogActionHandler);
    }

    void HumanControl_Sv::onSpawned()
    {
        auto &obj = getLevelObject();

    	Logger::verbose() << "Spawned Human Control at "
    			<< obj.getPosition().x << "/"
				<< obj.getPosition().y << "/"
				<< obj.getPosition().z
                << " (bounding box " << obj.getBoundingBox().min().x << "/" << obj.getBoundingBox().min().y << "/" << obj.getBoundingBox().min().z
                << " to " << obj.getBoundingBox().max().x << "/" << obj.getBoundingBox().max().y << "/" << obj.getBoundingBox().max().z << ")";

    	glm::vec3 playerLookDirection = glm::eulerAngles(obj.getRotation());
    	mPitch = playerLookDirection.x;
    	mYaw = playerLookDirection.y;

        obj.setupRenderingAndPhysics(od::ObjectRenderMode::NOT_RENDERED, od::ObjectPhysicsMode::SOLID);
        obj.setupSkeleton();

        auto animPlayer = obj.getSkeletonAnimationPlayer();
        if(animPlayer != nullptr)
        {
            mCharacterController = std::make_shared<odPhysics::CharacterController>(getServer().getPhysicsSystem(), obj.getPhysicsHandle(), obj, 0.1, 0.4);

            animPlayer->setBoneAccumulator(mCharacterController, 0);
            animPlayer->setBoneModes({ odAnim::BoneMode::NORMAL,
                                       odAnim::BoneMode::NORMAL,
                                       odAnim::BoneMode::NORMAL }, 0);

            // played through the object, so clients get to know about it (they play animations on their own skeletons)
            odAnim::AnimModes animModes;
            animModes.playbackType = odAnim::PlaybackType::LOOPING;
            auto readyAnim = mFields.readyAnim.getAsset();
            if(readyAnim != nullptr)
            {
                Logger::verbose() << "Player starts out with animation '" << readyAnim->getName() << "'";
                obj.playAnimation(readyAnim, animModes);
            }

        }else
        {
            Logger::warn() << "Used Human Control class on object without skeleton";
        }

    	obj.setEnableUpdate(true);
    }

    void HumanControl_Sv::onUpdate(float relTime)
    {
        //auto guard = getServer().compensateLag(mClientId);
        auto &obj = getLevelObject();

        obj.setRotation(glm::quat(glm::vec3(0, mYaw, 0)));
        float yawSpeed = (mYaw - mLastUpdatedYaw)/relTime;
        mLastUpdatedYaw = mYaw;

        auto animPlayer = obj.getSkeletonAnimationPlayer();

        // handle state transitions that might happen during update
        bool swimming = (mCharacterController != nullptr) && mCharacterController->isSwimming();
        bool underwater = swimming && mCharacterController->isUnderwater();
        if(swimming != mWasSwimming || underwater != mWasUnderwater)
        {
            // entered or left the water, or dived or surfaced. pick the matching animation for what we are doing
            Logger::verbose() << "Player " << (swimming ? (underwater ? "dives" : "swims at the surface") : "stops swimming");
            mWasSwimming = swimming;
            mWasUnderwater = underwater;
            _playMovementAnim();
        }

        if(mCharacterController != nullptr)
        {
            // swimming is not driven by the animation's motion but by the speeds the class defines. the character swims where it looks,
            //  so looking down dives and looking up surfaces
            float verticalSpeed = 0.0f;
            if(swimming && (mState == State::RunningForward || mState == State::RunningBackward))
            {
                float speed = (mState == State::RunningForward) ? mFields.swimForwardSpeed.get() : mFields.swimBackwardSpeed.get();
                float pitch = (std::fabs(mPitch) < SWIM_PITCH_DEADZONE) ? 0.0f : mPitch;
                mCharacterController->moveRelative(glm::vec3(0, 0, -1)*speed*std::cos(pitch)*relTime, relTime);
                verticalSpeed = speed*std::sin(pitch);
            }
            mCharacterController->setSwimVerticalSpeed(verticalSpeed);

            // air. the class defines the rates in percent per second
            if(underwater)
            {
                mAir = std::max(0.0f, mAir - mFields.airConsumptionRate.get()*relTime);
                if(mAir <= 0.0f && !mDrowning)
                {
                    // TODO: this should kill the player. there is no health yet, so we pull the player up instead
                    Logger::info() << "Player ran out of air";
                    mDrowning = true;
                }

            }else
            {
                mAir = std::min(100.0f, mAir + mFields.airReplenishmentRate.get()*relTime);
                if(mDrowning && mAir > 0.0f)
                {
                    mDrowning = false;
                }
            }
            mCharacterController->setForceSurface(mDrowning && swimming);
        }

        switch(swimming ? State::Idling : mState)
        {
        case State::Idling:
        case State::TurningLeft:
        case State::TurningRight:
            if(swimming)
            {
                break;
            }

            if(yawSpeed >= TURN_ANIM_THRESHOLD)
            {
                _playAnim(mFields.turnLeft, true, false);
                mState = State::TurningLeft;

            }else if(yawSpeed <= -TURN_ANIM_THRESHOLD)
            {
                _playAnim(mFields.turnRight, true, false);
                mState = State::TurningRight;

            }else if(mState != State::Idling)
            {
                if(animPlayer != nullptr && animPlayer->isPlaying())
                {
                    break; // wait till turn anim is done
                }

                _playAnim(mFields.readyAnim, true, false);
                mState = State::Idling;
            }
            break;

        default:
            break;
        }

        if(mCharacterController != nullptr)
        {
            mCharacterController->update(relTime);

            mLogTimer += relTime;
            if(mLogTimer > 2.0f)
            {
                mLogTimer = 0.0f;
                auto pos = getLevelObject().getPosition();
                Logger::verbose() << "Player at " << pos.x << " " << pos.y << " " << pos.z << (mCharacterController->isOnGround() ? " (on ground)" : " (in the air)");
            }
        }
    }

    void HumanControl_Sv::_handleAction(Action action, odInput::ActionState state)
    {
        // TODO: we probably have to add the packet and view latency to these state changes
        float clientLag = getServer().getEstimatedClientLag(mClientId); // skip animations ahead by this time

        if(state == odInput::ActionState::BEGIN)
        {
            switch(action)
            {
            case Action::Forward:
                mState = State::RunningForward;
                _playMovementAnim(clientLag);
                break;

            case Action::Backward:
                mState = State::RunningBackward;
                _playMovementAnim(clientLag);
                break;

            case Action::Attack_Primary:
                _playAnim(mFields.oneHandRH, false, false, clientLag);
                _attack();
                break;

            default:
                break;
            }

        }else
        {
            mState = State::Idling;
            _playMovementAnim(clientLag);
        }
    }

    void HumanControl_Sv::_playMovementAnim(float skipAhead)
    {
        bool swimming = (mCharacterController != nullptr) && mCharacterController->isSwimming();

        if(swimming)
        {
            switch(mState)
            {
            case State::RunningForward:
                _playSwimAnim(mFields.swim);
                break;

            case State::RunningBackward:
                _playSwimAnim(mFields.backswim);
                break;

            default:
                _playSwimAnim((mCharacterController != nullptr && mCharacterController->isUnderwater()) ? mFields.idleUnderwater : mFields.treadWater);
                break;
            }

        }else
        {
            switch(mState)
            {
            case State::RunningForward:
                _playAnim(mFields.runAnim, false, true, skipAhead);
                break;

            case State::RunningBackward:
                _playAnim(mFields.runBackwards, false, true, skipAhead);
                break;

            default:
                _playAnim(mFields.readyAnim, true, true, skipAhead);
                break;
            }
        }
    }

    void HumanControl_Sv::_playSwimAnim(const odRfl::AnimRef &animRef)
    {
        // the motion comes from the swim speeds, so the root motion of the animation is dropped (it would move the mesh otherwise)
        odAnim::AnimModes modes;
        modes.playbackType = odAnim::PlaybackType::LOOPING;
        modes.boneModes = { odAnim::BoneMode::IGNORE, odAnim::BoneMode::NORMAL, odAnim::BoneMode::IGNORE };

        auto animPlayer = getLevelObject().getSkeletonAnimationPlayer();
        auto anim = animRef.getAsset();
        if(animPlayer != nullptr && anim != nullptr)
        {
            Logger::verbose() << "Player plays swim animation '" << anim->getName() << "'";
            getLevelObject().playAnimation(anim, modes);
            animPlayer->setBoneModes(modes.boneModes, 0);
        }
    }

    void HumanControl_Sv::_handleAnalogAction(Action action, const glm::vec2 &pos)
    {
        if(action == Action::Look)
        {
            glm::vec2 yawPitch = TrackingCamera_Cl::cursorPosToYawPitch(pos);
            mYaw = yawPitch.x;
            mPitch = yawPitch.y;
        }
    }

    void HumanControl_Sv::_attack()
    {
        Logger::info() << "Trying to attack...";

        auto &obj = getLevelObject();
        auto pos = obj.getPosition();
        odPhysics::ContactTestResultVector results;
        getServer().getPhysicsSystem().sphereTest(pos, 1.0, odPhysics::PhysicsTypeMasks::LevelObject, results);

        for(auto &result : results)
        {
            odPhysics::ObjectHandle *objectHandle = result.handle->asObjectHandle();
            if(objectHandle != nullptr && objectHandle->getLevelObject().getObjectId() != obj.getObjectId())
            {
                Logger::info() << "I, " << obj.getObjectId() << ", attacked " << objectHandle->getLevelObject().getObjectId();
            }
        }
    }

    void HumanControl_Sv::_playAnim(const odRfl::AnimRef &animRef, bool skeletonOnly, bool looping, float skipAheadTime)
    {
        odAnim::AnimModes modes;
        modes.playbackType = looping ? odAnim::PlaybackType::LOOPING : odAnim::PlaybackType::NORMAL;
        if(skeletonOnly)
        {
            modes.boneModes = { odAnim::BoneMode::NORMAL, odAnim::BoneMode::NORMAL, odAnim::BoneMode::NORMAL };

        }else
        {
            modes.boneModes = { odAnim::BoneMode::ACCUMULATE, odAnim::BoneMode::NORMAL, odAnim::BoneMode::ACCUMULATE};
        }

        auto animPlayer = getLevelObject().getSkeletonAnimationPlayer();
        auto anim = animRef.getAsset();
        if(animPlayer != nullptr && anim != nullptr)
        {
            Logger::verbose() << "Player plays animation '" << anim->getName() << "'" << (looping ? " (looping)" : "");

            // played through the object, so clients get to know about it (they play animations on their own skeletons)
            getLevelObject().playAnimation(anim, modes);
            animPlayer->setBoneModes(modes.boneModes, 0);
            if(skipAheadTime > 0)
            {
                animPlayer->update(skipAheadTime);
            }
        }
    }


    HumanControl_Cl::HumanControl_Cl()
    {
    }

    HumanControl_Cl::~HumanControl_Cl()
    {
    }

    void HumanControl_Cl::onLoaded()
    {
        odRfl::PrefetchProbe probe(getLevelObject().getClass()->getDependencyTable());
        mFields.probeFields(probe);

        getLevelObject().setSpawnStrategy(od::SpawnStrategy::Always);

        auto analogActionHandler = std::bind(&HumanControl_Cl::_handleAnalogAction, this, std::placeholders::_1, std::placeholders::_2);
        auto &lookAction = getClient().getInputManager().getAnalogAction(Action::Look);
        lookAction.addCallback(analogActionHandler);
    }

    void HumanControl_Cl::onSpawned()
    {
        auto &obj = getLevelObject();

    	obj.setupRenderingAndPhysics(od::ObjectRenderMode::NORMAL, od::ObjectPhysicsMode::SOLID);
        obj.setupSkeleton();

        auto animPlayer = obj.getSkeletonAnimationPlayer();
        if(animPlayer != nullptr)
        {
            odAnim::AnimModes modes;
            modes.playbackType = odAnim::PlaybackType::LOOPING;
            animPlayer->playAnimation(mFields.readyAnim.getAsset(), modes);
        }

        // create a tracking camera for me
        std::vector<std::shared_ptr<od::LevelObject>> foundCameras;
        obj.getLevel().findObjectsOfType(TrackingCamera::classId(), foundCameras);
        if(foundCameras.empty())
        {
            Logger::error() << "Found no Tracking Camera in level! This could be an error in level design.";

        }else
        {
            if(foundCameras.size() > 1)
            {
                Logger::warn() << "More than one Tracking Camera found in level! Ignoring all but one";
            }

            auto &cameraObject = *foundCameras.back();
            auto trackingCameraInstance = od::downcast<TrackingCamera_Cl>(cameraObject.getClassInstance());
            trackingCameraInstance->setObjectToTrack(&obj);
        }
    }

    void HumanControl_Cl::onTransformChanged()
    {
        // note: the sound listener is moved by the tracking camera, as the player hears from where the camera is
    }

    void HumanControl_Cl::onUpdate(float relTime)
    {
    }

    void HumanControl_Cl::_handleAnalogAction(Action action, const glm::vec2 &pos)
    {
        if(action == Action::Look)
        {
            // while the menu is open, the mouse is not for looking around
            if(getRfl<DragonRfl>().isMenuMode())
            {
                return;
            }

            // TODO: we have to mark this as a prediction, and have to shift the server side calculation so it roughly matches up
            //  with this state update
            glm::vec2 yawPitch = TrackingCamera_Cl::cursorPosToYawPitch(pos);
            getLevelObject().setRotation(glm::quat(glm::vec3(0, yawPitch.x, 0)));
        }
    }


    HumanControlDummy_Cl::HumanControlDummy_Cl()
    {
    }

    HumanControlDummy_Cl::~HumanControlDummy_Cl()
    {
    }

    void HumanControlDummy_Cl::onSpawned()
    {
    	getLevelObject().setupRenderingAndPhysics(od::ObjectRenderMode::NORMAL, od::ObjectPhysicsMode::SOLID);
    }

}

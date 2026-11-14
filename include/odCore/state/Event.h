
/*
 * @file Event.h
 * @author zal
 *
 * An Event is something whose execution (on either the server or the client)
 * can be delayed for whatever reason. They have a distinct, known time of
 * occurrence and a target (object, subsystem or whatever). Events must not be
 * abstract. They themselves can only be used for engine-domain stuff, although
 * they may carry RFL-domain data (like messages).
 */

#ifndef INCLUDE_ODCORE_STATE_EVENT_H_
#define INCLUDE_ODCORE_STATE_EVENT_H_

#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include <odCore/IdTypes.h>
#include <odCore/Message.h>

#include <odCore/anim/AnimModes.h>

#include <odCore/db/Asset.h>
#include <odCore/db/Animation.h>

#include <odCore/input/Action.h>

namespace od
{
    class LevelObject;
    class DataReader;
    class DataWriter;
}

namespace odDb
{
    class Animation;
    class Sound;
}

namespace odState
{

    /**
     * @brief Empty base for event classes so a visitor can implement a catch-all functor.
     *
     * These are not supposed to be virtual. Dispatch should be static based on a variant over all event types.
     * Make sure to keep these as lightweigt as possible.
     */
    struct Event
    {
    };


    struct ActionEvent final : public Event
    {
        ActionEvent(odInput::ActionCode code, bool down);
        ActionEvent(od::DataReader &dr);

        void serialize(od::DataWriter &dw) const;

        odInput::ActionCode actionCode;
        bool keyDown;
    };


    struct ObjectAnimEvent final : public Event
    {
        ObjectAnimEvent(od::LevelObjectId obj, const odDb::GlobalAssetRef &anim, const odAnim::AnimModes &modes);
        ObjectAnimEvent(od::DataReader &dr);

        void serialize(od::DataWriter &dw) const;

        od::LevelObjectId objectId;
        odDb::GlobalAssetRef animRef;
        std::shared_ptr<odDb::Animation> anim;
        odAnim::AnimModes modes;
    };


    struct ObjectMessageEvent final : public Event
    {
        ObjectMessageEvent(od::LevelObjectId sender, od::LevelObjectId receiver, const od::Message &msg);
        ObjectMessageEvent(od::DataReader &dr);

        void serialize(od::DataWriter &dw) const;

        od::LevelObjectId senderObjectId;
        od::LevelObjectId receiverObjectId;
        od::Message message;
    };


    /**
     * @brief Plays a sound at the position of an object. Only has an effect on clients.
     */
    struct ObjectSoundEvent final : public Event
    {
        ObjectSoundEvent(od::LevelObjectId obj, const odDb::GlobalAssetRef &sound, float volume);
        ObjectSoundEvent(od::DataReader &dr);

        void serialize(od::DataWriter &dw) const;

        od::LevelObjectId objectId;
        odDb::GlobalAssetRef soundRef;
        std::shared_ptr<odDb::Sound> sound; // filled in by the event queue when the event is received
        float volume; // linear, 0..1
    };


    /**
     * @brief Switches the background music. Only has an effect on clients.
     */
    struct MusicEvent final : public Event
    {
        MusicEvent(uint32_t musicId, float fadeInTime, float fadeOutTime);
        MusicEvent(od::DataReader &dr);

        void serialize(od::DataWriter &dw) const;

        uint32_t musicId;
        float fadeInTime;
        float fadeOutTime;
    };


    // NOTE: The order of type parameters determines the type discriminant,
    //        which is part of the network protocol! So always add types at the
    //        end, and instead of removing one, replace it with a dummy instead.
    using EventVariant = std::variant<ActionEvent,
                                      ObjectAnimEvent,
                                      ObjectMessageEvent,
                                      ObjectSoundEvent,
                                      MusicEvent>;


    struct EventVariantSerializer
    {
        static void serialize(const EventVariant &e, od::DataWriter &dw);
        static EventVariant deserialize(od::DataReader &dr);
    };


}

#endif

/*
 * DefaultObjectClass.h
 */

#ifndef INCLUDE_ODCORE_RFL_DEFAULTOBJECTCLASS_H_
#define INCLUDE_ODCORE_RFL_DEFAULTOBJECTCLASS_H_

#include <odCore/rfl/Class.h>

namespace odRfl
{

    /**
     * @brief A field bundle with no fields, for objects whose RFL class is not implemented.
     */
    class DefaultObjectFields final : public FieldBundle
    {
    public:

        virtual void probeFields(FieldProbe &probe) override { }

    };


    /**
     * @brief Server side of the fallback class for level objects whose RFL class could not be found. Does nothing.
     */
    class DefaultObject_Sv final : public ServerClass, public SpawnableClass, public ClassImpl<DefaultObject_Sv>
    {
    public:

        virtual FieldBundle &getFields() override { return mFields; }

        virtual void onSpawned() override;


    private:

        DefaultObjectFields mFields;

    };


    /**
     * @brief Client side of the fallback class for level objects whose RFL class could not be found.
     *
     * This is a fallback that should only be used as long as OpenDrakan's RFL does not define all RFL classes.
     * It makes sure that objects which are only referenced by other things (like items used by cutscenes) exist.
     * Since we can't tell items from objects that are never supposed to be seen (sound sources, triggers and other
     * markers that are only visible in the editor), these objects start out hidden. Whatever uses them (e.g. a cutscene
     * attaching an item to a character) has to make them visible. They don't have any physics, so they can't get in the way of anything.
     */
    class DefaultObject_Cl final : public ClientClass, public SpawnableClass, public ClassImpl<DefaultObject_Cl>
    {
    public:

        virtual FieldBundle &getFields() override { return mFields; }

        virtual void onSpawned() override;


    private:

        DefaultObjectFields mFields;

    };


    using DefaultObjectClassFactory = DefaultClassFactory<DefaultObjectFields, DefaultObject_Cl, DefaultObject_Sv>;

}

#endif /* INCLUDE_ODCORE_RFL_DEFAULTOBJECTCLASS_H_ */

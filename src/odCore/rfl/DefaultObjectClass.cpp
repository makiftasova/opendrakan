/*
 * DefaultObjectClass.cpp
 */

#include <odCore/rfl/DefaultObjectClass.h>

#include <odCore/LevelObject.h>
#include <odCore/ObjectStates.h>

namespace odRfl
{

    static void _hideUntracked(od::LevelObject &obj)
    {
        // untracked, since client and server do the same thing and thus start out with the same state
        od::ObjectStates states;
        states.visibility = false;
        obj.setStatesUntracked(states);
    }

    void DefaultObject_Sv::onSpawned()
    {
        _hideUntracked(getLevelObject());
    }

    void DefaultObject_Cl::onSpawned()
    {
        auto &obj = getLevelObject();

        obj.setupRenderingAndPhysics(od::ObjectRenderMode::NORMAL, od::ObjectPhysicsMode::NO_PHYSICS);

        _hideUntracked(obj);
    }

}

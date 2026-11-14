/*
 * RflDragon.h
 *
 *  Created on: 16 Sep 2018
 *      Author: zal
 */

#ifndef INCLUDE_DRAGONRFL_RFLDRAGON_H_
#define INCLUDE_DRAGONRFL_RFLDRAGON_H_

#include <memory>
#include <set>

#include <odCore/input/InputManager.h>

#include <odCore/net/IdTypes.h>
#include <odCore/net/MessageDispatcher.h>

#include <odCore/rfl/Rfl.h>

#include <dragonRfl/Actions.h>
#include <dragonRfl/MessageChannel.h>

namespace dragonRfl
{

    class LocalPlayer;
    class DragonGui;

    class DragonRfl : public odRfl::Rfl
    {
    public:

        DragonRfl();
        virtual ~DragonRfl();

        inline LocalPlayer *getLocalPlayer() { return mLocalPlayer; }
        inline void setLocalPlayer(LocalPlayer *lp) { mLocalPlayer = lp; }

        /**
         * @brief Spawns a controller for the given client.
         */
        void spawnHumanControlForPlayer(od::Server &localServer, odNet::ClientId client);

        virtual const char *getName() const override;
        virtual void onLoaded() override;
        virtual void onGameStartup(od::Server &localServer, od::Client &localClient, bool loadIntroLevel) override;
        virtual void onLevelLoaded(od::Server &localServer) override;

        void setShowMenu(bool b);

        /**
         * @brief Starts a new game by loading the first level (as specified by the interface database) on the local server.
         *
         * Can be called from any thread, but only after onGameStartup().
         */
        void startNewGame();


    private:

        void _bindActions(odInput::InputManager &im);
        void _handleAction(Action action, odInput::ActionState state);

        // note: this is defined in ClassRegistry.cpp, as to avoid having to include all the class headers in the main RFL logic code
        void _registerClasses();

        LocalPlayer *mLocalPlayer;
        od::Server *mLocalServer;
        std::set<odNet::ClientId> mClientsWithSkipBinding;
        od::Client *mLocalClient;
        std::unique_ptr<DragonGui> mGui;

        std::shared_ptr<odNet::GlobalMessageListener<MessageChannel>> mControlCreationChannelListener;
    };

}

#endif /* INCLUDE_DRAGONRFL_RFLDRAGON_H_ */

#pragma once

#include "../util/version.hpp"
#include "../manager/audioManager.hpp"
#include "../manager/gameManager.hpp"
#include "../manager/levelManager.hpp"
#include "../manager/settingsManager.hpp"
#include "../manager/saveDataManager.hpp"
#include "../manager/hudManager.hpp"
#include "../network/websocketClient.hpp"
#include "../network/messageAliases.hpp"
#include "../facade/hyperengine/hyperEngine.hpp"

class HUDSystem;

class GameContext
{
public:
    GameContext(
        GraphicsEngine& graph
        ,AudioManager& audio
        ,EntityManager&entMan
        ,GameManager& gameman
        ,LevelManager& lvlMan
        ,SettingsManager& setMan
        ,SaveDataManager& saveMan
        ,HUDManager& hudMan
        ,NetworkClient& netwCli
        ,WebMessageInterface& webmsg
    )
    : graphicsEngine(graph)
    , audioManager(audio)
    , entityManager(entMan)
    , gameManager(gameman)
    , levelManager(lvlMan)
    , settingsManager(setMan)
    , saveDataManager(saveMan)
    , hudManager(hudMan)
    , networkClient(netwCli)
    , webMessageInterface(webmsg)
    {
        this->webMessageInterface.setGameContext(this);
        this->networkClient.msgInterface = &webMessageInterface;
        this->hudManager.setGameContext(this);
    }
    ~GameContext() {}

    GraphicsEngine& graphicsEngine;
    AudioManager& audioManager;
    EntityManager& entityManager;
    GameManager& gameManager;
    LevelManager& levelManager;
    SettingsManager& settingsManager;
    SaveDataManager& saveDataManager;
    HUDManager& hudManager;
    NetworkClient& networkClient;
    WebMessageInterface& webMessageInterface;
private:
};

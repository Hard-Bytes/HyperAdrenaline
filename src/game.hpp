#pragma once
#include "context/gameContext.hpp"
#include "network/webMessageInterface.hpp"
#include "components/component.hpp"
#include "system/allSystemsInclude.hpp"
#include "util/gameTimer.hpp"

#include "manager/stateManager.hpp" // use to push states and call to update them

class Game
{
public:
    // Constructor
    Game();
    ~Game();

    void start();
    void gameInit();
    void gameRun();
    void gameClose();
private:
    HyperEngine         graphicsEngine;
    AudioManager        audioManager;
    NetworkClient       networkClient;
    SettingsManager     settingsManager;
    SaveDataManager     saveDataManager;
    EntityManager       entityManager;
    HUDManager          hudManager;
    GameManager         gameManager{entityManager, graphicsEngine, audioManager, hudManager, networkClient, settingsManager, saveDataManager};
    LevelManager        levelManager{entityManager, gameManager, graphicsEngine, audioManager, networkClient, settingsManager};
    WebMessageInterface webMessageInterface;

    GameContext gameContext {
        graphicsEngine
        ,audioManager
        ,entityManager
        ,gameManager
        ,levelManager
        ,settingsManager
        ,saveDataManager
        ,hudManager
        ,networkClient
        ,webMessageInterface
    };
};

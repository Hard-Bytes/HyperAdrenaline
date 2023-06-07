#include "gameState.hpp"
#include "../util/macros.hpp"
#include "../util/paths.hpp"
#include "decisionState.hpp"

GameState::GameState(StateManager& sm, GameContext& gc, int desiredLevel, bool wantsOnline)
    : stateman(sm)
    , gctx(gc)
    , m_startingLevelIndex(desiredLevel)
    , m_wantsOnline(wantsOnline)
{}

GameState::~GameState()
{}

void
GameState::init()
{
    // Hide cursor
    gctx.graphicsEngine.hideCursor(true);

    m_usePhysicsOcclusion = gctx.settingsManager.get<bool>("debug/usePhysicsOcclusion");

    // HUD elements init
    gctx.hudManager.resetData();
    gctx.hudManager.setActiveHUD(&this->hudSys);
    auto playerLook = gctx.audioManager.getPlayerLook();
    if(playerLook == PL_Liam)
        this->hudSys.loadFromFile(UTIL_HUDOVERLAY_GAME_BLUE);
    else
        this->hudSys.loadFromFile(UTIL_HUDOVERLAY_GAME_GREEN);
    this->hudSys.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    // Create multiplayer HUD stuff
    gctx.hudManager.makeOtherPlayerData(0);
    gctx.hudManager.makeOtherPlayerData(1);
    gctx.hudManager.makeOtherPlayerData(2);
    gctx.hudManager.setOtherPlayerInfoVisible(0, false);
    gctx.hudManager.setOtherPlayerInfoVisible(1, false);
    gctx.hudManager.setOtherPlayerInfoVisible(2, false);

    // Load the first level
    gctx.levelManager.loadLevel(m_startingLevelIndex);

    // Force delta reset
    gctx.graphicsEngine.resetTimeDIff();

    // First online update (mainly for entity vinculation)
    // Also enable entity spawning
    gctx.webMessageInterface.setAllowEntitySpawning(true);
    gctx.webMessageInterface.processAllMessages();
}

void
GameState::resume()
{
    // Rescale hud
    this->hudSys.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());

    // Hide cursor
    gctx.graphicsEngine.hideCursor(true);

    gctx.hudManager.setActiveHUD(&this->hudSys);
    m_paused = false;

    if(m_backFromEndGameDecision)
    {
        gctx.levelManager.flagLevelFinished = false;
        m_backFromEndGameDecision = false;
        if(m_endGameDecisionBackToMenu)
            gctx.levelManager.flagGoBackToSelection = true;
        else
            gctx.levelManager.flagGoToNextLevel = true;

        // Check flags
        endGameChecks();
    }
}

void
GameState::update()
{
    if (!m_paused || m_wantsOnline)
    {
        // Timer Loop
        auto localPlayerId = gctx.gameManager.getLocalPlayerID();
        if(localPlayerId >= 0)
        {
            auto playerNode = gctx.entityManager.getComponentByID<NodeComponent>(localPlayerId);
            auto position = playerNode->node->getPosition();
            gctx.audioManager.setListener3DPosition(position);
        } 
        gctx.audioManager.update();

        if(!m_paused)
        {
            // If we are online but not connected to a server, go back
            if(m_wantsOnline && !gctx.networkClient.isOnline())
            {
                m_alive = false;
                bool tmp;
                std::string disconnectionReason { gctx.webMessageInterface.getDisconnectionReason() };
                gctx.webMessageInterface.resetDisconnectionReason();
                stateman.pushState<DecisionState>(gctx, tmp, UTIL_HUDOVERLAY_SELECTION_DISCONNECTED, disconnectionReason);
                return;
            }
            // Check close key
            if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
            {
                stateman.pushState<PauseState>(stateman, gctx, m_alive);
                m_paused = true;
                return;
            }
        }

        // Update dynamic lights
        gctx.gameManager.updateDynamicLightning();
        gctx.graphicsEngine.updateAllAnimations();

        // Main loop
        if(!m_paused) inputSys.updateAll();
        if(gctx.networkClient.isHost()) aiSys.updateAll();
        behaviourSys.updateAll();
        collisionSys.updateAll();
        weaponSys.updateAll();
        statusSys.updateAll();
        if(!m_paused) particleSys.updateAll();
        if(m_usePhysicsOcclusion) gctx.levelManager.updateOcclusionByPhysics();
        if(!m_paused) renderSys.updateAll(true, gctx.gameManager.showUI);
        healthSys.updateAll();
        hudSys.updateAll();
        if(gctx.networkClient.isHost()) respawnSys.respawnEnemies();
        if(gctx.networkClient.isHost()) raidSys.respawnEnemies();
        gctx.webMessageInterface.processAllMessages();

        // Check game end
        endGameChecks();

        // Physics updating
        gctx.graphicsEngine.updatePhysics();
        
        // Update dead entities
        gctx.gameManager.updateDeadEntities();
        gctx.entityManager.updateDeadEntities();
    }
    else
    {
        gctx.graphicsEngine.resetTimeDIff();
    }
}

void
GameState::close()
{
    // Reset level index in server
    if(gctx.networkClient.isHost())
        gctx.networkClient.send("LevelFinished");

    // Make hud's pointer null
    gctx.hudManager.setActiveHUD(nullptr);

    // Stop entity spawning via online
    gctx.webMessageInterface.setAllowEntitySpawning(false);

    // Stop audio
    gctx.audioManager.stopAllAudio();
    gctx.audioManager.update();

    // Recover mouse
    gctx.graphicsEngine.hideCursor(false);

    // Clear all entities
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();

    // Free map resources
    //gctx.graphicsEngine.freeResource(
    //    gctx.levelManager.getLastMapLoaded()
    //);
}

bool 
GameState::isAlive() {
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void GameState::endGameChecks() noexcept
{
    if(gctx.levelManager.flagLevelFinished && !m_backFromEndGameDecision)
    {
        gctx.levelManager.flagLevelFinished = false;
        m_backFromEndGameDecision = true;
        stateman.pushState<DecisionState>(gctx, m_endGameDecisionBackToMenu, UTIL_HUDOVERLAY_SELECTION_LEVELFINISHED);
        
        gctx.networkClient.send("LevelFinished");
    }
    else if(gctx.levelManager.flagGoBackToSelection)
    {
        gctx.levelManager.flagGoBackToSelection = false;
        m_alive = false;
    }
    else if(gctx.levelManager.flagGoToNextLevel)
    {  
        auto& levelMan = gctx.levelManager;
        gctx.gameManager.m_menuSelection = levelMan.getNextLevelIndex();
        gctx.gameManager.m_selectionAccepted = true;//gctx.networkClient.isHost();
        levelMan.flagGoToNextLevel = false;
        m_alive = false;
        return;
    }
    else if(gctx.levelManager.flagPlayerDied || gctx.levelManager.flagReloadLevel)
    {
        auto& levelMan = gctx.levelManager;
        gctx.gameManager.m_menuSelection = levelMan.getCurrentLevelIndex();
        gctx.gameManager.m_selectionAccepted = true;//gctx.networkClient.isHost();
        levelMan.flagPlayerDied = false;
        levelMan.flagReloadLevel = false;
        m_alive = false;
        return;
    }
}

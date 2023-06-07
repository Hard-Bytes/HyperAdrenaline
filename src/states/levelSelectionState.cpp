#include "levelSelectionState.hpp"
#include "gameState.hpp"
#include "decisionState.hpp"
#include "../manager/levelDataAliases.hpp"
#include "../util/jsonReader.hpp"

LevelSelectionState::LevelSelectionState(StateManager& sm, GameContext& gc, bool wantsOnline) 
    : m_statemanager(sm)
    , gctx(gc)
    , m_wantsOnline(wantsOnline)
{}

void
LevelSelectionState::init()
{
    // Clear scene from main menu
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();

    // Load HUD
    gctx.hudManager.setActiveHUD(&this->hud);
    this->hud.loadFromFile(UTIL_HUDOVERLAY_LEVEL_SELECTION);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    // Load levels filenames
    rapidjson::Document levelOrderDoc;
    openJsonFile(UTIL_MAP_DATA_FOLDER UTIL_MAP_ORDER_FILE, levelOrderDoc);
    this->m_maxLevels = (int)levelOrderDoc.Size();

    std::vector<std::string> levelNames;
    levelNames.reserve(m_maxLevels);
    for(int i=0; i<m_maxLevels; ++i)
    {
        std::string mapName = levelOrderDoc[i].GetString();
        rapidjson::Document doc;
        openJsonFile(UTIL_MAP_DATA_FOLDER+mapName, doc);
        std::string newName { doc[MAP_SECTION_STRUCTURE][MAP_DATA_MAPNAME].GetString() };
        levelNames.emplace_back("["+std::to_string(i+1)+"] "+newName);
    }
    gctx.gameManager.m_menuSelection = -1;
    gctx.gameManager.m_selectionAccepted = false;
    if(!gctx.networkClient.isHost()) setListColorGray();
    gctx.hudManager.makeList("list_levels", levelNames);
    gctx.hudManager.setActiveHUD(nullptr);

    // Recover connection data
    if(m_wantsOnline)
    {
        auto host { gctx.settingsManager.getString("multiplayer/host") };
        auto port { gctx.settingsManager.getString("multiplayer/port") };
        auto playerName { gctx.settingsManager.getString("multiplayer/playerName") };
        gctx.gameManager.setPlayerName(playerName);
        gctx.gameManager.addPlayerNameToList(playerName);

        // Connect to the server
        gctx.networkClient.connect("ws://"+host+":"+port);
        // Obviously remove this shit later, wait for connection if the game started online
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Get player look
        PlayerLook look { PL_Liam };
        if(!gctx.networkClient.isHost()) look = PL_Captain;
        gctx.audioManager.setPlayerLook(look);

        gctx.networkClient.send("PlayerConnected" SEP
            + playerName
            + SEP + std::to_string(look)
            + SEP + std::to_string(gctx.networkClient.isHost())
        );
        gctx.networkClient.send("GetPlayerList");
    }
    // If we are online, set host to true
    // This is because the game will do some stuff only if we are the host
    // Also, singleplayer is always Liam (for now?)
    else
    {
        auto playerName { gctx.settingsManager.getString("multiplayer/playerName") };
        gctx.gameManager.setPlayerName(playerName);
        gctx.audioManager.setPlayerLook(PL_Liam);
        gctx.networkClient.setHost(true);
    }
}

void
LevelSelectionState::resume()
{
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
    
    // Only reset the music if we are coming back from the game
    // This avoids music resetting when coming back from a modal like "Do you want to leave?"
    if(m_backFromGame)
    {
        // Set background music
        gctx.audioManager.stopAllAudioOfType(SOUNDS_MUSIC);
        gctx.audioManager.playSound(SOUND_MUSIC_MAIN_MENU, SOUNDS_MUSIC);
    }
    else
    {
        m_backFromGame = true;
    }

    // If we are resuming and we have been disconnected, player would have seen the disconection message in the game already
    // So skip it and die
    if(m_wantsOnline && !gctx.networkClient.isOnline())
        m_alive = false;
}

void
LevelSelectionState::update()
{
    gctx.audioManager.update();
    renderSys.renderLevelSelection(
        m_wantsOnline
        , gctx.gameManager.getPlayerNames()
    );

    // If we are online but not connected to a server, go back
    if(m_wantsOnline && !gctx.networkClient.isOnline())
    {
        m_alive = false;
        bool tmp;
        std::string disconnectionReason { gctx.webMessageInterface.getDisconnectionReason() };
        gctx.webMessageInterface.resetDisconnectionReason();
        m_statemanager.pushState<DecisionState>(gctx, tmp, UTIL_HUDOVERLAY_SELECTION_DISCONNECTED, disconnectionReason);
        return;
    }

    bool isHost = gctx.networkClient.isHost();
    gctx.webMessageInterface.processAllMessages();
    if(gctx.gameManager.m_menuSelection >= 0)
    {
        m_selectedLevel = gctx.gameManager.m_menuSelection;
        gctx.gameManager.m_menuSelection = -1;
        repositionateArrow();
    }

    // Key checking
    if(isHost && gctx.graphicsEngine.getKeyKeyboardPress(Key::DOWN))
    {
        m_selectedLevel = (m_selectedLevel + 1) % m_maxLevels;
        repositionateArrow();
        gctx.networkClient.send("SelectionChanged" SEP + std::to_string(m_selectedLevel));
    }
    else if(isHost && gctx.graphicsEngine.getKeyKeyboardPress(Key::UP))
    {
        --m_selectedLevel;
        if(m_selectedLevel < 0)
            m_selectedLevel = m_maxLevels - 1;
        repositionateArrow();
        gctx.networkClient.send("SelectionChanged" SEP + std::to_string(m_selectedLevel));
    }
    // Enter accepts the current selection, go to game
    else if((isHost && gctx.graphicsEngine.getKeySinglePress(Key::ENTER)) || gctx.gameManager.m_selectionAccepted)
    {
        gctx.gameManager.m_selectionAccepted = false;
        m_statemanager.pushState<GameState>(m_statemanager, gctx, m_selectedLevel, m_wantsOnline);
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
    {
        // Show a modal if we are online, or close directly if we are not online
        // This avoids the player disconnecting if they didn't know they'll disconnect
        if(m_wantsOnline)
        {
            m_backFromGame = false;
            m_statemanager.pushState<DecisionState>(gctx, m_alive, UTIL_HUDOVERLAY_SELECTION_EXITONLINE);
        }
        else
        {
            m_alive = false;
        }
    }

    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void
LevelSelectionState::close()
{
    // Disconnect
    gctx.networkClient.close("Game closed");

    // Clear scene
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
LevelSelectionState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void LevelSelectionState::repositionateArrow()
{
    auto sel = hud.getHUDElement("list_levels_"+std::to_string(m_selectedLevel));
    auto arrow = hud.getHUDElement("text_arrow");
    auto pos = arrow->getRelativePosition();
    pos.y = sel->getRelativePosition().y;
    arrow->setRelativePosition(pos);
}

void LevelSelectionState::setListColorGray()
{
    auto* element { static_cast<HUDElementListBase*>(hud.getHUDElement("list_levels")) };
    element->setTextColor(hud.getColorFromPalette("disabledTextColor"));
}

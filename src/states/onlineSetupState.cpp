#include "onlineSetupState.hpp"
#include "levelSelectionState.hpp"

OnlineSetupState::OnlineSetupState(StateManager& sm, GameContext& gc) 
: m_statemanager(sm), gctx(gc)
{}

void
OnlineSetupState::init()
{ 
    m_playerName    = gctx.settingsManager.getString("multiplayer/playerName");
    m_hostToConnect = gctx.settingsManager.getString("multiplayer/host");

    // Init hud
    this->hud.loadFromFile(UTIL_HUDOVERLAY_ONLINE_SETUP);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));
    setHUDNameAndIP();
}

void
OnlineSetupState::resume()
{
    // When we come back from other menus, skip this
    m_alive = false;
}

void
OnlineSetupState::update()
{
    gctx.audioManager.update();
    renderSys.updateAll(false);
    
    behaviourSystem.updateAll();

    if(gctx.graphicsEngine.getKeyKeyboardPress(Key::DOWN))
    {
        m_selection = (m_selection + 1) % m_maxSelection;
        repositionArrow();
    }
    else if(gctx.graphicsEngine.getKeyKeyboardPress(Key::UP))
    {
        --m_selection;
        if(m_selection < 0)
            m_selection = m_maxSelection - 1;
        repositionArrow();
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ENTER))
    {
        switch (m_selection)
        {
        case 0:
            gctx.networkClient.setHost(true);
            m_statemanager.pushState<LevelSelectionState>(m_statemanager, gctx, true);
            break;
        case 1:
            gctx.networkClient.setHost(false);
            m_statemanager.pushState<LevelSelectionState>(m_statemanager, gctx, true);
            break;
        }
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
    {
        m_alive = false;
    }

    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void
OnlineSetupState::close()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
OnlineSetupState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void OnlineSetupState::setHUDNameAndIP()
{
    auto* welcome = static_cast<HUDElementText*>(hud.getHUDElement("text_greeting"));
    auto* ip = static_cast<HUDElementText*>(hud.getHUDElement("text_destination"));

    welcome->setText("Welcome, "+m_playerName+"!");
    ip->setText(ip->getText() + m_hostToConnect);
}

void OnlineSetupState::repositionArrow()
{
    auto sel = hud.getHUDElement("text_option"+std::to_string(m_selection));
    auto arrow = hud.getHUDElement("text_arrow");
    auto pos = arrow->getRelativePosition();
    pos.y = sel->getRelativePosition().y;
    arrow->setRelativePosition(pos);
}

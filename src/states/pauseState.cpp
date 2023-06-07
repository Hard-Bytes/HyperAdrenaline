#include "pauseState.hpp"
#include "optionsState.hpp"

PauseState::PauseState(StateManager& sm, GameContext& gc, bool& p_parentIsAlive)
: m_statemanager(sm), gctx(gc), m_parentIsAlive(p_parentIsAlive)
{
    isTransparent = true;
}

void 
PauseState::init()
{
    gctx.graphicsEngine.hideCursor(false);
    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();

    this->hud.loadFromFile(UTIL_HUDOVERLAY_PAUSE_MENU);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));
}

void
PauseState::resume()
{
    m_usingAnotherState = false;
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
}

void 
PauseState::update()
{
    if(m_usingAnotherState) return;
    renderSys.updateAll();

    gctx.audioManager.update();

    if(gctx.graphicsEngine.getKeyKeyboardPress(Key::DOWN))
    {
        m_selection = (m_selection + 1) % m_maxSelection;
        repositionateArrow();
    }
    else if(gctx.graphicsEngine.getKeyKeyboardPress(Key::UP))
    {
        --m_selection;
        if(m_selection < 0)
            m_selection = m_maxSelection - 1;
        repositionateArrow();
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
    {
        m_alive = false;
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ENTER))
    {
        switch (m_selection)
        {
        case 0:
            m_alive = false;
            break;
        case 1:
            m_usingAnotherState = true;
            m_statemanager.pushState<OptionsState>(gctx, true);
            break;
        case 2:
            m_alive = false;
            m_parentIsAlive = false;
            break;
        case 3:
            gctx.graphicsEngine.closeMainDevice();
            break;
        }
    }

    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void 
PauseState::close()
{
    gctx.graphicsEngine.repositionMouseCenter();
    gctx.graphicsEngine.resetTimeDIff();
}

bool 
PauseState::isAlive() {
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void PauseState::repositionateArrow()
{
    auto sel = hud.getHUDElement("text_option"+std::to_string(m_selection));
    auto arrow = hud.getHUDElement("text_arrow");
    auto pos = arrow->getRelativePosition();
    pos.y = sel->getRelativePosition().y;
    arrow->setRelativePosition(pos);
}

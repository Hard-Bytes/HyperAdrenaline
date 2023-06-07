#include "decisionState.hpp"

DecisionState::DecisionState(
    GameContext& gc
    , bool& responseStorage
    , const std::string& datafile
    , const std::string& extramessage
    , bool updatesUnderlyingState
) 
    : gctx(gc)
    , m_datafile(datafile)
    , m_extramessage(extramessage)
    , m_responseStorage(responseStorage)
{
    isTransparent = updatesUnderlyingState;
}

void
DecisionState::init()
{
    // Initialize this hud
    this->hud.loadFromFile(m_datafile);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    // Locate options
    int counter = 0;
    while(true)
    {
        auto sel = hud.getHUDElement("text_option"+std::to_string(counter));
        if(!sel) break;
        counter++;
    }
    m_nElements = counter;

    // If no options found, return
    if(m_nElements == 0)
    {
        m_alive = false;
        return;
    }

    // Load answer values
    m_optionValues.reserve(m_nElements);
    m_optionValues.push_back(true);
    if(m_nElements > 1)
        m_optionValues.push_back(false);

    // Repositionate arrow to first option
    repositionateArrow();

    // Patch for disconnecion reason
    if(m_extramessage != "")
    {
        auto* elem = static_cast<HUDElementText*>(hud.getHUDElement("text_option0"));
        elem->setText("Ok ("+m_extramessage+")");
    }
}

void
DecisionState::resume()
{
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
}

void
DecisionState::update()
{
    gctx.audioManager.update();
    renderSys.updateAll(false);
    auto& graph = gctx.graphicsEngine;

    if(graph.getKeyKeyboardPress(Key::DOWN))
    {
        m_selection = (m_selection + 1) % m_nElements;
        repositionateArrow();
    }
    else if(graph.getKeyKeyboardPress(Key::UP))
    {
        if(--m_selection < 0) m_selection = m_nElements - 1;
        repositionateArrow();
    }
    else if(graph.getKeySinglePress(Key::ENTER))
    {
        m_responseStorage = m_optionValues[m_selection];
        m_alive = false;
    }

    graph.updateMouseControllerVariables();
    graph.updateKeyReleaseTrigger();
}

void
DecisionState::close()
{}

bool 
DecisionState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void DecisionState::repositionateArrow()
{
    auto sel = hud.getHUDElement("text_option"+std::to_string(m_selection));
    auto arrow = hud.getHUDElement("selection_arrow");
    auto pos = sel->getRelativePosition();
    pos.x -= 0.02f;
    arrow->setRelativePosition(pos);
}

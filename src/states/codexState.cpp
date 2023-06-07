#include "codexState.hpp"
#include "codexSectionState.hpp"
#include "creditsState.hpp"

CodexState::CodexState(StateManager& stam, GameContext& gc) 
: stateman(stam), gctx(gc)
{}

void
CodexState::init()
{
    this->hud.loadFromFile(UTIL_HUDOVERLAY_CODEX_MAIN);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    this->resume();

    m_selection = 1;
    repositionateArrow();
}

void
CodexState::resume()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
}

void
CodexState::update()
{
    gctx.audioManager.update();
    renderSys.updateAll(false);
    auto& graph = gctx.graphicsEngine;

    if(graph.getKeySinglePress(Key::ESC))
    {
        m_alive = false;
    }
    else if(graph.getKeySinglePress(Key::DOWN) || graph.getKeySinglePress(Key::RIGHT))
    {
        this->m_selection = (m_selection + 1) % m_maxSelection;
        repositionateArrow();
    }
    else if(graph.getKeySinglePress(Key::UP) || graph.getKeySinglePress(Key::LEFT))
    {
        if(--m_selection < 0)
            m_selection = m_maxSelection - 1;
        repositionateArrow();
    }
    else if(graph.getKeySinglePress(Key::ENTER))
    {
        switch(m_selection)
        {
            case 0:
                m_alive = false;
                break;
            case 1:
                stateman.pushState<CodexSectionState>(stateman, gctx, "Crewmates");
                break;
            case 2:
                stateman.pushState<CodexSectionState>(stateman, gctx, "Enemies");
                break;
            case 3:
                stateman.pushState<CodexSectionState>(stateman, gctx, "Weapons");
                break;
            case 4:
                stateman.pushState<CreditsState>(gctx);
                break;
            case 5:
                stateman.pushState<CodexSectionState>(stateman, gctx, "ShipSections");
                break;
            case 6:
                stateman.pushState<CodexSectionState>(stateman, gctx, "ShipEquipment");
                break;
        }
    }
    /*
        else if(graph.getKeySinglePress(Key::UP))
        {
            // 0            -> -
            // 1, 2, 3 y 4  -> =  0
            // 5 y 6        -> -= 3
        }
        else if(graph.getKeySinglePress(Key::DOWN))
        {
            // 0            -> =  4
            // 1, 2         -> =  5
            // 3            -> -= 6
            // 4, 5 y 6     -> -
        }
        else if(graph.getKeySinglePress(Key::LEFT))
        {
            // 0            -> =  3
            // 2, 3, 4 y 6  -> -= 1
            // 1 y 5        -> -
        }
        else if(graph.getKeySinglePress(Key::RIGHT))
        {
            // 0            -> =  3
            // 1, 2, 3 y 5  -> += 1
            // 6            -> =  4
            // 4            -> -
        }
    */

    graph.updateMouseControllerVariables();
    graph.updateKeyReleaseTrigger();
}

void
CodexState::close()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
CodexState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

void CodexState::repositionateArrow()
{
    auto sel = hud.getHUDElement("text_option"+std::to_string(m_selection));
    auto arrow = hud.getHUDElement("selection_arrow");
    auto pos = sel->getRelativePosition();
    pos.x -= 0.02f;
    arrow->setRelativePosition(pos);
}

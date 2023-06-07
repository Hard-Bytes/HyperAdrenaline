#include "codexSectionState.hpp"
#include "codexPageState.hpp"

CodexSectionState::CodexSectionState(StateManager& stam, GameContext& gc, const std::string& sectionFile) 
: stateman(stam), gctx(gc)
{}

void
CodexSectionState::init()
{
    this->hud.loadFromFile(UTIL_HUDOVERLAY_CODEX_SECTION);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    this->resume();
}

void
CodexSectionState::resume()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
}

void
CodexSectionState::update()
{
    gctx.audioManager.update();
    renderSys.updateAll(false);

    if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
    {
        m_alive = false;
    }
    else if(gctx.graphicsEngine.getKeySinglePress(Key::ENTER))
    {
        stateman.pushState<CodexPageState>(gctx, "Liam");
    }

    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void
CodexSectionState::close()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
CodexSectionState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

#include "codexPageState.hpp"

CodexPageState::CodexPageState(GameContext& gc, const std::string& pageFile) 
: gctx(gc)
{}

void
CodexPageState::init()
{
    this->hud.loadFromFile(UTIL_HUDOVERLAY_CODEX_PAGE);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    this->resume();
}

void
CodexPageState::resume()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

void
CodexPageState::update()
{
    gctx.audioManager.update();
    renderSys.updateAll(false);

    if(gctx.graphicsEngine.getKeySinglePress(Key::ESC))
    {
        m_alive = false;
    }

    gctx.graphicsEngine.updateMouseControllerVariables();
    gctx.graphicsEngine.updateKeyReleaseTrigger();
}

void
CodexPageState::close()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
CodexPageState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

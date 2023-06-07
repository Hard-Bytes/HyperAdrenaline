#include "creditsState.hpp"

CreditsState::CreditsState(GameContext& gc) 
: gctx(gc)
{}

void
CreditsState::init()
{
    this->hud.loadFromFile(UTIL_HUDOVERLAY_CREDITS);
    this->hud.setHUDScale(gctx.settingsManager.get<float>("ui/uiScale"));

    this->resume();
}

void
CreditsState::resume()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
    this->hud.updateAbsoluteData(gctx.graphicsEngine.getScreenResolution());
}

void
CreditsState::update()
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
CreditsState::close()
{
    gctx.gameManager.clearGame();
    gctx.graphicsEngine.clearScene();
}

bool 
CreditsState::isAlive() 
{
    return m_alive && gctx.graphicsEngine.isDeviceRunning();
}

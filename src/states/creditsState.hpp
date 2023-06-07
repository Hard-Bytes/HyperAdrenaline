#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"

struct CreditsState : public State
{
    explicit CreditsState(GameContext& gc);
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    bool m_alive { true };
};

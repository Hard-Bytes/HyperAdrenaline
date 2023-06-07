#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"

struct CodexState : public State
{
    explicit CodexState(StateManager& stam, GameContext& gc);
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    // Helpers
    void repositionateArrow();

    StateManager& stateman;
    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    int m_selection {0};
    int m_maxSelection {7};

    bool m_alive { true };
};

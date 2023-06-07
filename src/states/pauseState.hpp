#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"

struct PauseState : public State
{
    explicit PauseState(StateManager& sm, GameContext&, bool& p_parentIsAlive);
    ~PauseState() {};
    void init()   final;
    void resume() final;
    void update() final;
    void close()  final;
    bool isAlive()final;
private:
    // Helpers
    void repositionateArrow();

    StateManager& m_statemanager;
    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    int m_selection {0};
    int m_maxSelection {4};

    bool m_usingAnotherState { false };

    bool m_alive { true };
    bool& m_parentIsAlive;
};



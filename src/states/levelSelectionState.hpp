#pragma once
#include "../manager/stateManager.hpp"
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"

struct LevelSelectionState : public State
{
    explicit LevelSelectionState(StateManager&, GameContext&, bool wantsOnline);
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    // Helpers
    void repositionateArrow();
    void setListColorGray();

    StateManager& m_statemanager;
    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    int m_selectedLevel {0};
    int m_maxLevels {0};

    bool m_backFromGame { true };
    bool m_wantsOnline;
    bool m_alive { true };
};

#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"
#include "../system/behaviour.hpp"

struct MenuState : public State
{
    explicit MenuState(StateManager& sm, GameContext& gc);
    ~MenuState() = default;
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    // Helpers
    void repositionateArrow();

    StateManager& m_statemanager;
    GameContext& gctx;

    BehaviourSystem behaviourSystem {gctx};
    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    int cameraId { -1 };

    int m_selection {0};
    int m_maxSelection {5};

    bool m_alive { true };
};
 
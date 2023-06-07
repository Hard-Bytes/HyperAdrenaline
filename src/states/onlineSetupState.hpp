#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"
#include "../system/behaviour.hpp"

struct OnlineSetupState : public State
{
    explicit OnlineSetupState(StateManager& sm, GameContext& gc);
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    // Helpers
    void setHUDNameAndIP();
    void repositionArrow();

    StateManager& m_statemanager;
    GameContext& gctx;

    BehaviourSystem behaviourSystem {gctx};
    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    std::string m_playerName;
    std::string m_hostToConnect;

    int m_selection {0};
    int m_maxSelection {2};

    bool m_alive { true };
};
 
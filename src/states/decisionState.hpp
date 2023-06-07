#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"

struct DecisionState : public State
{
    explicit DecisionState(
        GameContext& gc
        , bool& responseStorage
        , const std::string& datafile
        , const std::string& extramessage = ""
        , bool updatesUnderlyingState = false
    );
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    // Helpers
    void repositionateArrow();

    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    std::string m_datafile;
    std::string m_extramessage;
    int m_selection { 0 };
    int m_nElements { 1 };

    std::vector<bool> m_optionValues;

    bool& m_responseStorage;
    bool m_alive { true };
};

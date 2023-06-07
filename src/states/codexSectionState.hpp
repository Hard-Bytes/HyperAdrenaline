#pragma once
#include "../manager/stateManager.hpp"
#include "../context/gameContext.hpp"
#include "../system/render.hpp"

struct CodexSectionState : public State
{
    explicit CodexSectionState(StateManager& stam, GameContext& gc, const std::string& sectionFile);
    void init()    final;
    void resume()  final;
    void update()  final;
    void close()   final;
    bool isAlive() final;
private:
    StateManager& stateman;
    GameContext& gctx;

    HUDSystem hud {gctx};
    RenderSystem renderSys {gctx, hud};

    bool m_alive { true };
};

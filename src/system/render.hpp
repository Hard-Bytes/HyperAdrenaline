#pragma once
#include "../context/gameContext.hpp"
#include "hud.hpp"
#include "particle.hpp"

class RenderSystem
{
public:
    // Constructor
    explicit RenderSystem(GameContext& gc, HUDSystem& hs, ParticleSystem* ps = nullptr);
    ~RenderSystem();

    void update(bool updateGameData = true, bool drawUI = true);
    void updateAll(bool updateGameData = true, bool drawUI = true);
    void renderLevelSelection(
        bool isOnline
        , const std::vector<std::string>& playerNames
    );
private:
    GameContext& gctx;
    HUDSystem& hud;
    ParticleSystem * particles {nullptr};
};

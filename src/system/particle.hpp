#pragma once
#include "../context/gameContext.hpp"

struct ParticleSystem
{
    explicit ParticleSystem(GameContext& gc);
    ~ParticleSystem();

    void updateOne(ParticleComponent& cmp);
    void updateAll(void);

    void renderOne(ParticleComponent& cmp);
    void renderAll(void);
private:
    GameContext& gctx;
};


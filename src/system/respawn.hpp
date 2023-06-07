#pragma once
#include "../context/gameContext.hpp"

#define MIN_DISTANCE 7
#define MAX_DISTANCE 400
class RespawnSystem
{
public:
    // Constructor
    explicit RespawnSystem(GameContext& gc);
    ~RespawnSystem();

    void respawnEnemies() const noexcept;
private:
    GameContext& gctx;
};

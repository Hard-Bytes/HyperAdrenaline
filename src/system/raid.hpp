#pragma once
#include "../context/gameContext.hpp"

#define MIN_DISTANCE_RAID 20
#define MAX_DISTANCE_RAID 100
class RaidSystem
{
public:
    // Constructor
    explicit RaidSystem(GameContext& gc);
    ~RaidSystem();

    void respawnEnemies();
private:
    GameContext& gctx;
    int enemiesSpawned=0;
    
};

#pragma once
#include "../context/gameContext.hpp"

class AISystem
{
public:
    // Constructor
    explicit AISystem(GameContext& gc);
    ~AISystem();

    void updateOne(AIComponent& cmp) const noexcept;
    void updateAll() const noexcept;
private:
    GameContext& gctx;
};

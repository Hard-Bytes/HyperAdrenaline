#pragma once
#include "../context/gameContext.hpp"

class HealthSystem
{
public:
    // Constructor
    explicit HealthSystem(GameContext& gc);
    ~HealthSystem();

    void updateOne(HealthComponent& cmp) const noexcept;
    void updateAll() const noexcept;
private:
    GameContext& gctx;
};

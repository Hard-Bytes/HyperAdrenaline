#pragma once
#include "../context/gameContext.hpp"

class BehaviourSystem
{
public:
    // Constructor
    explicit BehaviourSystem(GameContext& gc);
    ~BehaviourSystem();

    void updateOne(BehaviourComponent& cmp) const noexcept;
    void updateAll() const noexcept;
private:
    GameContext& gctx;
};

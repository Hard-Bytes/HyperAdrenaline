#pragma once
#include "../context/gameContext.hpp"
#include "../components/allComponentsInclude.hpp"

class StatusSystem
{
public:
    // Constructor
    explicit StatusSystem(GameContext& gc);
    ~StatusSystem();

    void updateOne(StatusComponent& cmp) const noexcept;
    void updateAll() const noexcept;
private:
    GameContext& gctx;
};

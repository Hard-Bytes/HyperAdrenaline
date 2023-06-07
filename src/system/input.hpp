#pragma once
#include "../context/gameContext.hpp"
#include "hud.hpp"

#define UMBRAL_SPEED 0.1

class InputSystem
{
public:
    // Constructor
    explicit InputSystem(GameContext& gc);
    ~InputSystem();

    void updateOne(InputComponent& cmp) const noexcept;
    void updateAll() const noexcept;
private:
    GameContext& gctx;

    void checkDebugInputs(InputComponent& cmp) const noexcept;
};

#pragma once
#include "healthCmp.hpp"
#include "../context/gameContext.hpp"

// cf = Component Functions
namespace cf
{
    bool doDamage(HealthComponent& cmp, GameContext& gctx, float damage, float cooldownCaused = 0.0f, bool sendOnline=true) noexcept;
    void kill(HealthComponent& cmp, GameContext& gctx, bool sendOnline=true) noexcept;
    bool isDead(HealthComponent& cmp) noexcept;
}

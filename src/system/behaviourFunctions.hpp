#pragma once

// Behaviours Functions
#include "../context/gameContext.hpp"
#include "../components/aiCmp.hpp"

#define BehaviourFunctionParams BehaviourComponent& cmp, GameContext& gctx
#define declareBehaviourFunction(func) void func(BehaviourFunctionParams)

declareBehaviourFunction(behPickup);
declareBehaviourFunction(behKillingMyselfByTime);
declareBehaviourFunction(behKillingMyselfByTimeAndCollision);
declareBehaviourFunction(behTriggerMyselfByCollision);
declareBehaviourFunction(behScalateMyselfAndDieByTime);

declareBehaviourFunction(behMenuCameraRotation);

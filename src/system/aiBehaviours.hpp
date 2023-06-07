#pragma once

// AI Behaviours
#include <math.h>
#include "../context/gameContext.hpp"
#include "../components/allComponentsInclude.hpp"
#include "../components/entity.hpp"
#include "../behaviours/behaviourTree.hpp"

#define AIFunctionParams AIComponent& cmp, GameContext& gctx
#define declareAIFunction(func) void func(AIFunctionParams)

declareAIFunction(aiPatroller);
declareAIFunction(aiMovingPlatform);
declareAIFunction(aiHealingPad);
declareAIFunction(aiAmmoPad);

void helperGenericPad(AIComponent& cmp, GameContext& gctx, int padType);
const Vector3f helperCalculateSteering(std::vector<SteeringBehaviour>&, const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition, GameContext& gctx);
const Vector3f helperSeek(const CollisionComponent& colcmp, const AIComponent& aicmp) noexcept;
const Vector3f helperArrive(const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition) noexcept;
const Vector3f helperFlee(const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition) noexcept;
const Vector3f helperSeparation(const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition, std::vector<Vector3f>& targets) noexcept;
const std::vector<Vector3f> helperGetOtherAIPosition(const EntityID& ownId, GameContext& gctx);

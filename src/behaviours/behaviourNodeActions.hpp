#pragma once
#include "behaviourStructures.hpp"

#define behNodeActionsParemeters TreeNode& treeNode, AIComponent& cmp,  GameContext& gctx
#define declareBehNodeActions(func) ResultType func(behNodeActionsParemeters)


// Composite nodes
declareBehNodeActions(behSelector);
declareBehNodeActions(behSequence);

// Decorator nodes
declareBehNodeActions(behSucceder);
declareBehNodeActions(behInverter);
declareBehNodeActions(behDebugNegator);

// Leaf nodes (behaviours)
declareBehNodeActions(behWalk);
declareBehNodeActions(behWalkFindPath);
declareBehNodeActions(behSeePlayer);
declareBehNodeActions(behPatrol);
declareBehNodeActions(behIsAlertOn);
declareBehNodeActions(behAlertOff);
declareBehNodeActions(behCloseTo);
declareBehNodeActions(behCanAttack);
declareBehNodeActions(behBeginAttack);
declareBehNodeActions(behIsAttacking);
declareBehNodeActions(behAttack);

declareBehNodeActions(behShoot);
declareBehNodeActions(behInRange);
declareBehNodeActions(behStalker);
declareBehNodeActions(behExplode);
declareBehNodeActions(behCanShoot);

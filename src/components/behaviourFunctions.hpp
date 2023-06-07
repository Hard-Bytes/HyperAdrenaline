#pragma once

// Forward declarations
class GameContext;
class InteractionSystem;
struct AIComponent;
struct BehaviourComponent;
struct InteractableComponent;
struct WeaponComponent;
struct Weapon;

// Behaviour Functions
typedef void (*AIBehaviourFunction)
    (AIComponent& cmp, GameContext& gctx);
typedef void (*BehaviourFunction)
    (BehaviourComponent& cmp, GameContext& gctx);
typedef void (*InteractionFunction)
    (InteractableComponent& cmp,
    int interactorID,
    GameContext& gctx);
typedef void (*WeaponFunction)
    (Weapon& weapon, WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition);
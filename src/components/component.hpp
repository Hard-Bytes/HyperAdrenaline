#pragma once
#include "../util/paths.hpp"
#include "../util/typeAliases.hpp"
#include "../facade/graphicsEngine.hpp"

enum ComponentType
{
    TypeAI = 0,
    TypeBehaviour,
    TypeCollision,
    TypeHealth,
    TypeInput,
    TypeInteractable,
    TypeInventory,
    TypeNode,
    TypeStatus,
    TypeType,
    TypeWeapon,
    TypeParticle
};

struct Component
{
public:
    // Constructor
    explicit Component(EntityID entID)
        : entityID(entID)
    {}
    virtual ~Component() = default;

    constexpr ComponentID getComponentID() const noexcept {return componentID;};
    constexpr EntityID getEntityID() const noexcept {return entityID;};
private:
    ComponentID componentID { ++nextID };
    EntityID entityID { 0 };
    inline static EntityID nextID { 0 };
};

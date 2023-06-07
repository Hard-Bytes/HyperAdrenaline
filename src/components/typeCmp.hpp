#pragma once
#include "component.hpp"
#include "../sharedTypes/serverEnums.hpp"

// Entity types
enum EntityType
{
    ENT_PLAYER = 0,
    ENT_ENEMY,
    ENT_OBSTACLE,
    ENT_PLATFORM,
    ENT_EVENTAREA,
    ENT_EFFECTAREA,
    ENT_ATTACKAREA,
    ENT_ENEMYATTACKAREA,
    ENT_EXPLOSIONAREA,
    ENT_UNLOCKEDDOOR,
    ENT_KILLDOOR,
    ENT_KEYDOOR,
    ENT_CREDITSDOOR,
    ENT_HEAL_VENDINGMACHINE,
    ENT_AMMO_VENDINGMACHINE,
    ENT_HEALKIT,
    ENT_AMMOKIT,
    ENT_PICKUPKEY,
    ENT_PICKUPWEAPON,
    ENT_OTHER
};

struct TypeComponent : public Component
{
    // Constructor
    explicit TypeComponent(EntityID entID, EntityType p_type);
    ~TypeComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeType; }

    // Data
    EntityType type;
    PlayerLook style;
    int idInServer{-1};
};

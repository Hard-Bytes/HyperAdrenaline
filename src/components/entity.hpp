#pragma once
#include "../util/typeAliases.hpp"
#include "allComponentsInclude.hpp"

class Entity
{
public:
    // Constructor
    Entity();
    ~Entity();

    constexpr EntityID getID() const noexcept {return entityID;};
    bool isDead{false};

    static void resetEntityIDAssigner() {nextID = 0;};
private:
    EntityID entityID { ++nextID };
    inline static EntityID nextID { 0 };
};

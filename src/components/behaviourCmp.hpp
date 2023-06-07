#pragma once
#include "component.hpp"
#include "behaviourFunctions.hpp"

struct BehaviourComponent : public Component
{
    // Constructor
    explicit BehaviourComponent(EntityID entID, BehaviourFunction behfunc=nullptr);
    ~BehaviourComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeBehaviour; } 
    BehaviourFunction behaviourFunction;

    Vector3f initialPos, finalPos;
    Vector3f lookAt;
    // Proyectile Sounds
    std::string sound {""};
    std::string soundExplosion {""};
};

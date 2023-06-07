#pragma once
#include "component.hpp"

// haciendo forward de la clase sistema de partículas en el hyperengine
namespace hyen {
    struct ParticleGenerator;
}

struct ParticleComponent : public Component
{
    ParticleComponent(EntityID entID);
    ~ParticleComponent();

    constexpr static ComponentID getComponentTypeID()
        {   return (ComponentID)TypeParticle;   }

    hyen::ParticleGenerator
        *generator          { nullptr }
    ,   *generatorHitmark   { nullptr }
    ;
    int priorityLevel {0};
    Vector3f offset {0,0,0};
};

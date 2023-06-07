#include "particleCmp.hpp"
#include "../util/macros.hpp"
#include <hyperengine/engine/particles/generator.hpp>

ParticleComponent::ParticleComponent(EntityID entID)
:   Component(entID)
{
}

ParticleComponent::~ParticleComponent()
{
    // delete generator;
    // generator = nullptr;

    // Ahora deleteado en el gameManager
}

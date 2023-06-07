#include "particle.hpp"
#include "../facade/hyperengine/hyperGraphicNode.hpp"
// #include <hyperengine/engine/particles/generator.hpp>

ParticleSystem::ParticleSystem(GameContext& gc)
:   gctx(gc)
{
}

ParticleSystem::~ParticleSystem()
{
}

void 
ParticleSystem::updateOne(ParticleComponent& cmp)
{
    // Hay que hacer el setMatrices
    auto* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());

    auto* camnode  = static_cast<hyen::Node*>(gctx.graphicsEngine.getActiveCameraHyperengine());
    if(!camnode) return;
    cmp.generator->setMatrices(camnode);
    if(cmp.generatorHitmark) cmp.generatorHitmark->setMatrices(camnode);

    if(nodeCmp)
    {
        auto nodepos { nodeCmp->node->getPosition() };
        glm::vec3 pos{ nodepos.makeGLMVector() };
        cmp.generator->setOrigin(pos + cmp.offset.makeGLMVector());
        // if(cmp.generatorHitmark) cmp.generatorHitmark->setOrigin(pos + offset);
    }

    auto const& dt { gctx.graphicsEngine.getTimeDiffInSeconds() };
    cmp.generator->update(dt);
    if(cmp.generatorHitmark) cmp.generatorHitmark->update(dt);
}

void 
ParticleSystem::updateAll(void)
{
    auto& components = gctx.entityManager.getComponentVector<ParticleComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}
    
void 
ParticleSystem::renderOne(ParticleComponent& cmp)
{
    int quality { gctx.graphicsEngine.getParticleQuality() };
    int prio { cmp.priorityLevel };
    if(prio > quality) return;
    
    cmp.generator->render();
    if(cmp.generatorHitmark) cmp.generatorHitmark->render();
}

void 
ParticleSystem::renderAll(void)
{
    auto& components = gctx.entityManager.getComponentVector<ParticleComponent>();
    for(auto& cmp : components)
    {
        this->renderOne(cmp);
    }
}

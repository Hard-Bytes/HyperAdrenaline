#include "ai.hpp"

// Constructor
AISystem::AISystem(GameContext& gc)
    : gctx(gc)
{}
// Destructor
AISystem::~AISystem(){}

/*Update One*/
void AISystem::updateOne(AIComponent& cmp) const noexcept
{
    if(cmp.aiFunction)
        cmp.aiFunction(cmp, this->gctx);
}

/*Update All*/
void AISystem::updateAll() const noexcept
{
    auto& components = gctx.entityManager.getComponentVector<AIComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}

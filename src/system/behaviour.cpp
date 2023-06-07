#include "behaviour.hpp"

// Constructor
BehaviourSystem::BehaviourSystem(GameContext& gc)
    : gctx(gc)
{}
// Destructor
BehaviourSystem::~BehaviourSystem(){}

/*Update One*/
void BehaviourSystem::updateOne(BehaviourComponent& cmp) const noexcept
{
    cmp.behaviourFunction(cmp, this->gctx);
}

/*Update All*/
void BehaviourSystem::updateAll() const noexcept
{
    auto& components = gctx.entityManager.getComponentVector<BehaviourComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}

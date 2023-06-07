#include "status.hpp"
#include "../components/healthCmpFunc.hpp"
#include "../components/inventoryCmpFunc.hpp"
#include "../util/macros.hpp"

// Constructor
StatusSystem::StatusSystem(GameContext& gc)
    : gctx(gc)
{}

// Destructor
StatusSystem::~StatusSystem()
{

}

/*Update One*/
void StatusSystem::updateOne(StatusComponent& cmp) const noexcept
{
    for(auto it = cmp.status.begin(); it != cmp.status.end(); ++it)
    {
        //Get delta time   
        float dt = gctx.graphicsEngine.getTimeDiffInSeconds();
        it->timeAtStat+=dt;

        // Only host calculates burning damage
        if(gctx.networkClient.isHost())
            if(STAT_BURNING==it->stat)
            {
                HealthComponent* healthComponent = gctx.entityManager.getComponentByID<HealthComponent>(cmp.getEntityID());
                if(it->timeAtStat/0.5 > it->statCounter)
                {
                    it->statCounter++;

                    float oldHealth=healthComponent->health;
                    cf::doDamage(*healthComponent, gctx, 0.5f);
                    if(it->statCause > 0  && oldHealth>0 && healthComponent->health <=0)
                    {
                        InteractableComponent* enemyInt = gctx.entityManager.getComponentByID<InteractableComponent>(cmp.getEntityID());
                        auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>(it->statCause);
                        cf::addCredits(*playerInv, enemyInt->sharedKeyId, gctx);
                    }
                }
            }

        if(it->timeAtStat>=it->maxTimeAtStat)
        {
            cmp.status.erase(it);
            --it;
        }
    }

}

/*Update All*/
void StatusSystem::updateAll() const noexcept
{
    auto& components = gctx.entityManager.getComponentVector<StatusComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}

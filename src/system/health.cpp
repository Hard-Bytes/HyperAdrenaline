#include "health.hpp"
#include "../components/healthCmpFunc.hpp"

// Constructor
HealthSystem::HealthSystem(GameContext& gc)
    : gctx(gc)
{}

// Destructor
HealthSystem::~HealthSystem()
{

}

/*Update One*/
void HealthSystem::updateOne(HealthComponent& cmp) const noexcept
{
    // Update inmunity cooldown
    float delta { gctx.graphicsEngine.getTimeDiffInSeconds() };
    if(cmp.inmunityCooldown > 0)
        cmp.inmunityCooldown -= delta;

    if(!cmp.totalDeathStarted && cf::isDead(cmp))
    {
        cmp.totalDeathStarted = true;
        int entityID { (int)cmp.getEntityID() };

        auto* typeCmp = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
        auto* aiCmp = gctx.entityManager.getComponentByID<AIComponent>(entityID);
        if(aiCmp && typeCmp && typeCmp->type == ENT_ENEMY)
        {
            auto* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(entityID);
            nodeCmp->node->playAnimation("death", false);
            gctx.graphicsEngine.disableAllCollisionsOf(nodeCmp->node);

            aiCmp->aiFunction = nullptr;

            auto* colCmp = gctx.entityManager.getComponentByID<CollisionComponent>(entityID);
            colCmp->moves = false;
            colCmp->collidesWithMap = false;
            colCmp->collidesWithEvents = false;
            colCmp->collidesWithAreaEffects = false;
            colCmp->speed = {0,0,0};
            colCmp->baseSpeed = {0,0,0};
            colCmp->acceleration = {0,0,0};

            auto* wpnCmp = gctx.entityManager.getComponentByID<WeaponComponent>(entityID);
            if(wpnCmp)
            {
                wpnCmp->weapons[wpnCmp->index].isFiring = false;
            }

            // Update enemy kills
            if(!aiCmp->isRaidEnemy)
            {
                ++gctx.gameManager.killedMapEnemies; //upoates the number of enemies killed
                gctx.hudManager.forceInteractableInfoRecalculation();
                // Send online
                if(gctx.networkClient.isHost())
                    gctx.networkClient.send(
                        "EnemiesKilledCountChanged" SEP
                        +std::to_string(gctx.gameManager.killedMapEnemies)
                    );
            }
            gctx.audioManager.playSound(SOUND_SFX_ENEMYDEATH, SOUNDS_SFX_COMBAT);
        }
    }
    else if(cmp.totalDeathStarted)
    {
        // Check timer
        cmp.totalDeathCounter -= delta;
        if(cmp.totalDeathCounter > 0) return;

        TypeComponent* typeCmp = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
        if(typeCmp && typeCmp->type == ENT_PLAYER)
        {
            // If it's a player, restart level
            gctx.levelManager.flagPlayerDied = true;
        }
        else
        {
            gctx.entityManager.markAsDead(cmp.getEntityID());
        }
    }
}

/*Update All*/
void HealthSystem::updateAll() const noexcept
{
    auto& components = gctx.entityManager.getComponentVector<HealthComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}

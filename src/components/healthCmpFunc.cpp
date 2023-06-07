#include "healthCmpFunc.hpp"
#include "statusCmp.hpp"

namespace cf
{
    bool doDamage(HealthComponent& cmp, GameContext& gctx, float damage, float cooldownCaused, bool sendOnline) noexcept
    {
        if(cmp.inmunityCooldown > 0 && damage > 0) 
            return false;
        if(damage == 0.f) cooldownCaused = 0.0f;

        StatusComponent* statusCmp = gctx.entityManager.getComponentByID<StatusComponent>(cmp.getEntityID());
        if(statusCmp)
        {
            for(unsigned int i=0; i<statusCmp->status.size(); i++)
            {
                if(statusCmp->status[i].stat==STAT_SLOWED)
                {
                    damage=damage*1.25;
                    i=statusCmp->status.size();
                }
            }
        }

        cmp.health -= damage;

        // Cap to max health
        cmp.health = std::min(cmp.health, cmp.maxHealth);
        cmp.inmunityCooldown = cooldownCaused;

        // Update HUD (different if player is local player or online player)
        int playerID { (int)cmp.getEntityID() };
        if(gctx.gameManager.getLocalPlayerID() == playerID)
        {
            gctx.hudManager.setLifebarLengthAndCount(cmp.health, cmp.maxHealth);
        }
        else
        {
            auto& players { gctx.gameManager.getPlayerIDs() };
            unsigned int i;
            for(i=1; i<players.size(); ++i)
                if(players[i] == playerID)
                    break;
            if(i < players.size())
                gctx.hudManager.setOtherPlayerLifebar(i-1, cmp.health, cmp.maxHealth);
        }

        if(sendOnline && damage != 0.f)
        {
            TypeComponent* type = gctx.entityManager.getComponentByID<TypeComponent>(playerID);
            gctx.networkClient.send(
                "EntityHealthChanged" SEP
                +std::to_string(type->idInServer)
                +SEP+std::to_string(damage)
            );
        }

        return true;
    }

    void kill(HealthComponent& cmp, GameContext& gctx, bool sendOnline) noexcept
    {
        cmp.health = 0.0f;

        if(sendOnline)
        {
            TypeComponent* type = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
            gctx.networkClient.send(
                "EntityHealthChanged" SEP
                +std::to_string(type->idInServer)
                +SEP+std::to_string(cmp.maxHealth)
            );
        }
    }

    bool isDead(HealthComponent& cmp) noexcept
    {
        return cmp.health <= 0.0f;
    }
}

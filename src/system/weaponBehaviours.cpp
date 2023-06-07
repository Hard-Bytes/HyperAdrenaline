#include "weaponBehaviours.hpp"
#include "../components/entity.hpp"
#include "../components/healthCmpFunc.hpp"
#include "../components/weaponCmpFunc.hpp"
#include "../components/inventoryCmpFunc.hpp"
#include "../util/macros.hpp"
#include "../system/hud.hpp"
#include <hyperengine/engine/hyperengine.hpp>
#include <math.h>

/*Weapon Shoot: Null*/
// Used for no-weapon, does nothing except maybe visual stuff (like sound)
void nullShoot(Weapon& weapon, WeaponComponent& ,GameContext& gctx, const Vector3f&)
{
    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        if( weapon.deltAcumulator >= weapon.shootingPeriod)
        {
            //cf::playWeaponSound(weapon, gctx);
            weapon.deltAcumulator = 0.f;
        }
    }
}

/*Weapon Shoot: Semiautomatic*/
// It fires everytime you press if isn't in cooldown
void basicShoot(Weapon& weapon,WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition)
{
    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        if( weapon.deltAcumulator >= weapon.shootingPeriod)
        {
            if(weapon.ammo)
            {
                if(weapon.ammo > 0) weapon.ammo--;
                // Play shooting sound if they have a sound with "charge it start charging"
                cf::playWeaponSound(weapon, gctx, shooterPosition);

                //Pick the deviation array acording with your selector that is a counter
                auto& vec=weapon.shootsDeviation[0][weapon.deviationSelector];
                //Shoots all shoots of that array
                for(auto deviate : vec ){
                    if(shootCalculations(deviate,weapon.damage,weapon.status,weapon.statusTime,cmp,gctx))
                        weapon.deviationSelector++;
                }
                weapon.deviationSelector++;
                //Check if you pass you max array
                while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation[0].size()){
                    weapon.deviationSelector-=weapon.shootsDeviation[0].size();
                }
                weapon.flagFiredShot = true;
                weapon.deltAcumulator = 0.f;
            }
            else
            {
                cf::stopWeaponSound(weapon, gctx);
                cf::playNoAmmoSound(weapon, gctx, shooterPosition);
            }
        }
    }
    else
    {
        cf::stopWeaponSound(weapon, gctx);
    }
}

/*Weapon Shoot:MachineGun*/
//It increase the cadency of the shoot every second
void aceleratedShoot(Weapon& weapon,WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition)
{

    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        float cadence = weapon.shootingPeriod-weapon.shootingTime*0.029;
        cadence = std::max(cadence, MINIMUN_CADENCE);
        if( weapon.deltAcumulator >= cadence) 
        {
            if(weapon.ammo)
            {
                if(weapon.ammo>0)
                {
                    weapon.ammo--;
                }
                // Play shooting sound
                cf::playWeaponSound(weapon, gctx, shooterPosition);
                gctx.audioManager.play3DSound(SOUND_SFX_WEAPON_MACHINEGUN_SHOOT,SOUNDS_SFX_COMBAT, shooterPosition);
                int chosenPackage=0;
                if(cadence <= 0.13)
                    chosenPackage=1;
                if(cadence == MINIMUN_CADENCE)
                    chosenPackage=2;

                while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation[chosenPackage].size()){
                    weapon.deviationSelector-=weapon.shootsDeviation[chosenPackage].size();
                }

                auto& vec=weapon.shootsDeviation[chosenPackage][weapon.deviationSelector];
                for(auto deviate : vec ){
                    if(shootCalculations(deviate,weapon.damage,weapon.status,weapon.statusTime,cmp,gctx))
                        weapon.deviationSelector++;
                }
                weapon.deviationSelector++;
                while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation[chosenPackage].size()){
                    weapon.deviationSelector-=weapon.shootsDeviation[chosenPackage].size();
                }
                weapon.shootingTime += cadence;
                weapon.flagFiredShot = true;
                weapon.deltAcumulator = 0.f;
            }else
            {
                cf::stopWeaponSound(weapon, gctx);  
                cf::playNoAmmoSound(weapon, gctx, shooterPosition);
            }
        }
    }
    else
    {
        cf::stopWeaponSound(weapon, gctx);
    }
}


/*Weapon Shoot: Sniper*/
// It charge the shoot and release the pawa
void sniperShoot(Weapon& weapon,WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition)
{

    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        if( weapon.deltAcumulator >= weapon.shootingPeriod) {
        if(weapon.ammo)
        {
            if(!weapon.flagFiredShot){
                weapon.ammo--;
                // Play shooting sound
                cf::playWeaponSound(weapon, gctx, shooterPosition);
                weapon.flagFiredShot = true;
                weapon.deltAcumulator = 0.f;
            }
        }else
        {
            cf::stopWeaponSound(weapon, gctx);  
            cf::playNoAmmoSound(weapon, gctx, shooterPosition);
        }
        }
    }else
    {   
        if(weapon.flagFiredShot){
            cf::stopWeaponSound(weapon, gctx);
            float trueDamage= weapon.damage+weapon.damage*2*weapon.deltAcumulator;
            auto& vec=weapon.shootsDeviation[0][weapon.deviationSelector];
            for(auto deviate : vec ){
                if(trueDamage>=weapon.damage*1.8)
                {
                    if(trueDamage>=weapon.damage*3)
                        trueDamage=weapon.damage*3;
                    if(shootCalculations(deviate,trueDamage,weapon.status,weapon.statusTime,cmp,gctx))
                        weapon.deviationSelector++;
                    shootStaticDamagingArea(
                        deviate
                        ,{1.5,1.5,500}
                        ,0
                        ,trueDamage*0.75
                        ,0
                        ,"" // Model (empty for cube)
                        ,"" // Sound (empty for silent)
                        ,cmp,gctx
                    );
                }else
                {
                    if(shootCalculations(deviate,weapon.damage,weapon.status,weapon.statusTime,cmp,gctx))
                        weapon.deviationSelector++;
                }
                
            }
            weapon.deviationSelector++;
            while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation[0].size()){
                weapon.deviationSelector-=weapon.shootsDeviation[0].size();
            }
            weapon.deltAcumulator = 0.f;
            weapon.flagFiredShot = false;
        }
    }
}

void retroShoot(Weapon& weapon,WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition)
{
    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        if( weapon.deltAcumulator >= weapon.shootingPeriod) {
          
        if(weapon.ammo)
        {
            weapon.ammo--;
            // Play shooting sound
            cf::playWeaponSound(weapon, gctx, shooterPosition);

            auto& vec=weapon.shootsDeviation[0][weapon.deviationSelector];
            for(auto deviate : vec ){
                shootStaticDamagingArea(
                    deviate
                    ,{2,2,2}
                    ,3
                    ,weapon.damage
                    ,17
                    ,MODEL_WEAPON_CANNON_BULLET
                    ,SOUND_SFX_WEAPON_CANNON_BULLET
                    ,cmp
                    ,gctx
                );
            }
            weapon.deviationSelector++;
            while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation.size()){
                weapon.deviationSelector-=weapon.shootsDeviation[0].size();
            }
            weapon.flagFiredShot = true;
            weapon.deltAcumulator = 0.f;
        }else
        {
            cf::stopWeaponSound(weapon, gctx);  
            cf::playNoAmmoSound(weapon, gctx, shooterPosition);
        }
        }
    }else
    {
        cf::stopWeaponSound(weapon, gctx);
    }
}


void explosiveShoot(Weapon& weapon,WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition)
{

    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        if( weapon.deltAcumulator >= weapon.shootingPeriod) {
          
            if(weapon.ammo)
            {
                weapon.ammo--;
                // Play shooting sound
                cf::playWeaponSound(weapon, gctx, shooterPosition);
                auto& vec=weapon.shootsDeviation[0][weapon.deviationSelector];
                for(auto deviate : vec ){
                    shootExplosionArea(
                        deviate
                        ,{0.8f, 0.8f, 0.8f}
                        ,weapon.damage
                        ,40
                        ,MODEL_WEAPON_EXPLOSIVE_BULLET
                        ,SOUND_SFX_WEAPON_EXPLOSIVE_BULLET
                        ,SOUND_SFX_WEAPON_EXPLOSIVE_BULLET_EXPLOSION
                        ,cmp, gctx
                    );
                }
                weapon.deviationSelector++;
                while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation.size()){
                    weapon.deviationSelector-=weapon.shootsDeviation.size();
                }
                weapon.flagFiredShot = true;
                weapon.deltAcumulator = 0.f;
            }else
            {
                cf::stopWeaponSound(weapon, gctx);  
                cf::playNoAmmoSound(weapon, gctx, shooterPosition);
            }
        }
    }else
    {
        cf::stopWeaponSound(weapon, gctx);
    }
}

/*Really Ugle Patch to make the shooting enemy shoot, because the fucking raycast*/
void enemyShoot(Weapon& weapon,WeaponComponent& cmp ,GameContext& gctx, const Vector3f& shooterPosition)
{
    weapon.deltAcumulator += gctx.graphicsEngine.getTimeDiffInSeconds();

    if(weapon.isFiring)
    {
        if( weapon.deltAcumulator >= weapon.shootingPeriod) 
        {
          
            if(weapon.ammo)
            {
                //weapon.ammo--;
                // Play shooting sound
                cf::playWeaponSound(weapon, gctx, shooterPosition);

                auto& vec=weapon.shootsDeviation[0][weapon.deviationSelector];
                for(auto deviate : vec)
                    shootEnemyBullet(deviate,{0.15f,0.15f,0.15f},40,SOUND_SFX_WEAPON_EXPLOSIVE_BULLET,cmp,gctx);
                    
                ++weapon.deviationSelector;
                while((uint32_t)weapon.deviationSelector>=weapon.shootsDeviation[0].size()){
                    weapon.deviationSelector-=weapon.shootsDeviation[0].size();
                }
                weapon.flagFiredShot = true;
                weapon.deltAcumulator = 0.f;

            }else
            {
                cf::stopWeaponSound(weapon, gctx);  
            }
        }
    }else
    {
        cf::stopWeaponSound(weapon, gctx);
    }
}

bool shootCalculations(Vector3f deviate,float damage,StatusType stat,float statTime,WeaponComponent& cmp ,GameContext& gctx){  
    // We use the camera target because the gunNode target core dumps
    NodeComponent* parentNode = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    
    // Get position and correct it if its not the local player (NPC players need to add the local offset)
    bool localPlayer = isLocalPlayer(cmp.getEntityID(), gctx);
    auto pos = parentNode->node->getPosition();
    if(!localPlayer) 
        pos += getLocalPlayerOffset(gctx);

    //New stuff for enemy shooters
    Vector3f targetVec = getTargetOf(cmp.getEntityID(), gctx);
    int mask = ~(MASK MASK_PLAYER);

    // int hit = gctx.graphicsEngine.checkRayCastCollisionWithNode(
    //     pos,
    //     (targetVec + deviate) - pos,
    //     1000.f,mask,true);

    Vector3f result;
    int hit = gctx.graphicsEngine.checkRayCastCollisionWithNodeAndResult(
        pos
    ,   (targetVec + deviate) - pos
    ,   1000.f
    ,   result
    ,   mask
    ,   true
    );

    // Check if it's an enemy
    TypeComponent* otherType = gctx.entityManager.getComponentByID<TypeComponent>(hit);
    TypeComponent* ourType = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
    // Get the target type and check if it we hit the target type
    EntityType target =
        (ourType && ourType->type==ENT_PLAYER) ? 
        ENT_ENEMY : ENT_PLAYER;

    if(otherType && otherType->type==target)
    {
        HealthComponent* healthComponent = gctx.entityManager.getComponentByID<HealthComponent>(hit);
        if(healthComponent)
        {
            float oldhealth = healthComponent->health;
            cf::doDamage(*healthComponent, gctx, damage);

            // partículas de golpe
            auto* pCmp = gctx.entityManager.getComponentByID<ParticleComponent>(hit);

            if(pCmp)
            {
                // Activa su generador, que es el de partículas de quemado
                glm::vec3 newOrigin{ result.x, result.y, result.z};
                pCmp->generatorHitmark->setOrigin( newOrigin );
                pCmp->generatorHitmark->setActive(true, true, 0.2f);
            }

			if(localPlayer) gctx.hudManager.setCrosshairEffect("crosshairHitmarker", "crosshairHitmark" );

            //Check if the weapon applies states
            int playerID = gctx.gameManager.getLocalPlayerID();
            if(stat!=STAT_NORMAL)
            {
                StatusComponent* statusComponent = gctx.entityManager.getComponentByID<StatusComponent>(hit);
                if(statusComponent)
                {
                    Stat statStruct = {stat,playerID,statTime};
                    statusComponent->status.push_back(statStruct);

                    // partículas de quemado
                    if(stat == STAT_BURNING)
                    {
                        if(pCmp)
                        {
                            // Activa su generador, que es el de partículas de quemado
                            pCmp->generator->setActive(true, true, statTime); // Son dos segundos por lo general
                        }
                    }

                    // Send online
                    auto hittedTypeCmp  = gctx.entityManager.getComponentByID<TypeComponent>(hit);
                    auto applierTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(playerID);
                    int applierID = INVALID_ENTITY;
                    if(applierTypeCmp) applierID = applierTypeCmp->idInServer;
                    if(hittedTypeCmp)
                        gctx.networkClient.send(
                            "StatusApplied" SEP
                            +std::to_string(hittedTypeCmp->idInServer)
                            +SEP+std::to_string((int)stat)
                            +SEP+std::to_string(statTime)
                            +SEP+std::to_string(applierID)
                        );
                }
            }

            auto hitNode = gctx.entityManager.getComponentByID<NodeComponent>(hit);
            if(hitNode)
                gctx.audioManager.play3DSound(SOUND_SFX_ENEMYDAMAGED, SOUNDS_SFX_COMBAT, hitNode->node->getPosition());

            //HP Checker to give you points
            if( oldhealth>0 && healthComponent->health <=0)
            {
                auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>(playerID);
                InteractableComponent* enemyInt = gctx.entityManager.getComponentByID<InteractableComponent>(hit);
                cf::addCredits(*playerInv, enemyInt->sharedKeyId, gctx);
            }
            return true;
        }
    }
    return false;
}

bool shootStaticDamagingArea(Vector3f deviate,Vector3f size,float time,float damage, float velocity, std::string model, std::string sound, WeaponComponent& cmp ,GameContext& gctx)
{  
    // We use the camera target because the gunNode target core dumps
    NodeComponent* parentNode = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());

    // Get position and correct it if its not the local player (NPC players need to add the local offset)
    bool localPlayer = isLocalPlayer(cmp.getEntityID(), gctx);
    auto pos = parentNode->node->getPosition();
    if(!localPlayer) 
        pos += getLocalPlayerOffset(gctx);
    
    Vector3f targetVec { getTargetOf(cmp.getEntityID(), gctx) };

    //Calculate the Angle that you need to rotate the item//
    Vector3f punto1 = pos;
    Vector3f punto2 = punto1 + ((targetVec + deviate) - pos) * size.x/2;
    
    Vector3f rotate { parentNode->node->getRotation() };
    Vector3f speed { punto2 - punto1 };
    speed = speed.normalize() * velocity;

    gctx.gameManager.createDamagingZonePerTimeEntity(punto2,size,rotate,speed,damage,time,false,model,sound);
    return false;
}

bool shootExplosionArea(Vector3f deviate,Vector3f size, float damage, float velocity, std::string model, std::string bulletSound, std::string explosionSound ,WeaponComponent& cmp ,GameContext& gctx)
{  
    // We use the camera target because the gunNode target core dumps
    NodeComponent* parentNode = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());

    // Get position and correct it if its not the local player (NPC players need to add the local offset)
    bool localPlayer = isLocalPlayer(cmp.getEntityID(), gctx);
    auto pos = parentNode->node->getPosition();
    if(!localPlayer) 
        pos += getLocalPlayerOffset(gctx);

    
    Vector3f targetVec = getTargetOf(cmp.getEntityID(), gctx);
    Vector3f rotate=Vector3f{0,0,0};

    //Calculate the Angle that you need to rotate the item//
    Vector3f punto1 = pos;
    Vector3f punto2 = punto1 + ((targetVec + deviate)- pos) * size.x/2;
    rotate=parentNode->node->getRotation();
    Vector3f speed = punto2-punto1;
    speed=speed.normalize()*velocity;

    gctx.gameManager.createExplosionMisilEntity(punto2,size,rotate,speed,damage,model,bulletSound,explosionSound);
    return false;
}

bool shootEnemyBullet(Vector3f deviate,Vector3f size,float velocity, std::string bulletSound ,WeaponComponent& cmp ,GameContext& gctx)
{  
    // We use the camera target because the gunNode target core dumps
    NodeComponent* parentNode = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());

    // Get position and correct it if its not the local player (NPC players need to add the local offset)
    auto pos = parentNode->node->getPosition();
    pos += cmp.weapons[cmp.index].offset;
    
    //Calculate the Angle that you need to rotate the item//
    auto* aiCmp = gctx.entityManager.getComponentByID<AIComponent>(cmp.getEntityID());
    NodeComponent* player = gctx.entityManager.getComponentByID<NodeComponent>(aiCmp->hittingPlayer);
        
    Vector3f punto1 { pos };
    Vector3f punto2 { player->node->getPosition() + deviate };
    Vector3f rotate { parentNode->node->getRotation() };

    Vector3f speed { punto2 - punto1 };
    speed = speed.normalize() * velocity;

    gctx.gameManager.createEnemyBullet(punto1,size,rotate,speed, cmp.weapons[cmp.index].damage, 3,bulletSound);

    return false;
}

Vector3f getTargetOf(EntityID entID, GameContext& gctx)
{
    // If it has AI, get the AI target
    AIComponent* aiCmp = gctx.entityManager.getComponentByID<AIComponent>(entID);
    if(aiCmp) return aiCmp->lookAt;

    // Else, get the target from the camera
    NodeComponent* parentNode = gctx.entityManager.getComponentByID<NodeComponent>(entID);
    return parentNode->node->getTarget();
}

bool isLocalPlayer(EntityID entID, GameContext& gctx)
{
    // True if local player is valid AND equal to the query id (entID)
    EntityID localPlayerId = gctx.gameManager.getLocalPlayerID();
    return localPlayerId >= 0 && entID == localPlayerId;
}

Vector3f getLocalPlayerOffset(GameContext& gctx)
{
    int localPlayerId = gctx.gameManager.getLocalPlayerID();
    if(localPlayerId < 0) return Vector3f();
    auto* cmp = gctx.entityManager.getComponentByID<CollisionComponent>(localPlayerId);
    if(!cmp) return Vector3f();
    return cmp->offset;
}

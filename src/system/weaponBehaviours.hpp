#pragma once

// Weapon Behaviours
#include "../components/allComponentsInclude.hpp"

#define MINIMUN_CADENCE 0.03f

#define declareWeaponFunction(func) void func(Weapon&,WeaponComponent&,GameContext&,const Vector3f&)

//
declareWeaponFunction(nullShoot);
declareWeaponFunction(basicShoot);
declareWeaponFunction(aceleratedShoot);
declareWeaponFunction(sniperShoot);
declareWeaponFunction(retroShoot);
declareWeaponFunction(explosiveShoot);
declareWeaponFunction(enemyShoot);


//Functions
bool shootCalculations(Vector3f deviate,float damage,StatusType stat,float statTime,WeaponComponent& cmp ,GameContext& gctx);
bool shootStaticDamagingArea(
    Vector3f deviate
    ,Vector3f size
    ,float time
    ,float damage
    ,float velocity
    ,std::string model
    ,std::string sound
    ,WeaponComponent& cmp
    ,GameContext& gctx
);
bool shootExplosionArea(
    Vector3f deviate
    ,Vector3f size
    ,float damage
    ,float velocity
    ,std::string model
    ,std::string bulletSound
    ,std::string explosionSound
    ,WeaponComponent& cmp
    ,GameContext& gctx
);
bool shootEnemyBullet(Vector3f deviate,Vector3f size,float velocity, std::string bulletSound,WeaponComponent& cmp ,GameContext& gctx);

// Helpers
Vector3f getTargetOf(EntityID entID, GameContext& gctx);
bool isLocalPlayer(EntityID entID, GameContext& gctx);
Vector3f getLocalPlayerOffset(GameContext& gctx);

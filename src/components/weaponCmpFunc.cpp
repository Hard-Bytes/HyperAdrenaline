#include "weaponCmpFunc.hpp"
#include "../system/weaponBehaviours.hpp"
namespace cf
{
    bool compareID(Weapon& a, Weapon& b) noexcept { return a.id < b.id; }

    void addWeapon(WeaponComponent& cmp, WeaponID id, GameContext* gctx,bool weaponPhrase) noexcept
    {
        // If you have it, add half of max ammo
        auto iter = std::find_if(cmp.weapons.begin(),cmp.weapons.end(),
            [id](Weapon &a){
                return a.id==id;
            });

        if(iter != cmp.weapons.end())
        {
            auto index = iter - cmp.weapons.begin();
            Weapon& gun = cmp.weapons[index];
            gun.ammo += 0.5f * gun.maxAmmo;

            if(gun.ammo > gun.maxAmmo)   
                gun.ammo = gun.maxAmmo;

            return;
        }

        // If we add a weapon, and we only had the fist before, delete the fist
        if(cmp.weapons.size() == 1 && cmp.weapons[0].id == W_EMPTYHAND)
            cmp.weapons.clear();
        
        // Else, create it
        RotationPackage shootDesv;
        Weapon newgun;

        switch (id)
        {
            case W_HANDGUN:
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_HANDGUN, SOUNDS_VOICE);
                
                shootDesv=
                {{
                    {{0,0,0}},{{0.015,0,0}},{{0,0.015,0.015}},{{-0.01,0,0.025}},{{0.025,0.025,0.025}},{{0.015,-0.025,-0.025}},{{-0.025,-0.01,-0.015}},{{0,0,0}},{{0,0.015,0}},{{-0.015,0.015,0}},{{-0.025,0,0.01}},{{0.015,-0.01,-0.01}},{{-0.015,-0.0015,-0.0015}},{{-0.01,-0.0015,-0.01}},{{0,0,0}},{{0,0,0.015}},{{0.015,0,-0.015}},{{0.01,-0.025,0.01}},{{-0.015,0.015,0.025}},{{-0.015,-0.01,-0.01}}
                }};
                cf::createWeapon(
                    newgun
                    ,W_HANDGUN
                    ,true
                    ,-1
                    ,400
                    ,3.f
                    ,0.f
                    ,MODEL_WEAPON_HANDGUN
                    //,{-1.f,-1.2f,2.f}
                    ,{1.f,-1.2f,-2.f}
                    ,SOUND_SFX_WEAPON_HANDGUN
                    ,false
                    ,shootDesv
                    ,basicShoot
                );
                break;
            case W_SHOTGUN:
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_SHOTGUN, SOUNDS_VOICE);
                shootDesv={{
                    {{0,0,0},{0.06,0,-0.06},{-0.06,0,0.06},{0.03,0.06,0.03},{-0.03,0.06,-0.03},{-0.03,-0.06,-0.03},{0.03,-0.06,0.03},{0.09,0.03,0.09},{-0.09,-0.03,-0.09},{-0.095,0.03,0.09},{-0.09,-0.03,0.095}}
                    ,{{0,0,0},{-0.06,0,0.06},{0.06,0,-0.06},{-0.03,-0.06,0.03},{-0.03,-0.06,-0.03},{0.03,0.06,0.03},{-0.03,0.06,-0.03},{-0.09,-0.03,-0.09},{0.09,0.03,0.09},{0.095,-0.03,-0.09},{0.09,0.03,-0.095}}
                    ,{{0,0,0},{-0.08,0,0.03},{-0.09,0,0.05},{0.04,0.08,0.07},{-0.07,-0.06,0.05},{-0.05,-0.05,-0.07},{0.09,-0.08,0.03},{0.04,0.04,-0.08},{-0.04,-0.09,-0.04},{-0.095,-0.05,0.06},{-0.06,-0.05,0.08}}
                    ,{{0,0,0},{0.03,0,-0.09},{-0.09,0,0.03},{-0.06,0.09,-0.06},{-0.06,0.03,-0.03},{-0.09,-0.03,-0.07},{0.06,-0.08,0.05},{0.03,0.06,0.09},{-0.09,-0.06,-0.03},{-0.095,0.055,0.075},{-0.085,-0.06,0.035}}
                    }};
                cf::createWeapon(
                    newgun
                    ,W_SHOTGUN
                    ,false
                    ,40
                    ,63
                    ,2.f
                    ,0.f
                    ,MODEL_WEAPON_SHOTGUN
                    ,{1.f,-0.9f,-2.f}
                    ,SOUND_SFX_WEAPON_SHOTGUN
                    ,false
                    ,shootDesv
                    ,basicShoot
                    ,STAT_BURNING
                    ,2.f);
                break;
            case W_MACHINEGUN:
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_MACHINEGUN, SOUNDS_VOICE);
                shootDesv=
                    {
                    { 
                    {{0,0,0}},{{0.015,0,0}},{{0,0.015,0.015}},{{-0.01,0,0.025}},{{0.025,0.025,0.025}},{{0.015,-0.025,-0.025}},{{-0.025,-0.01,-0.015}},{{0,0,0}},{{0,0.015,0}},{{-0.015,0.015,0}},{{-0.025,0,0.01}},{{0.015,-0.01,-0.01}},{{-0.015,-0.0015,-0.0015}},{{-0.01,-0.0015,-0.01}},{{0,0,0}},{{0,0,0.015}},{{0.015,0,-0.015}},{{0.01,-0.025,0.01}},{{-0.015,0.015,0.025}},{{-0.015,-0.01,-0.01}}
                    },
                    {
                    {{-0.05,-0.03,0.03}},{{0.03,0.03,-0.05}},{{0,0.015,0.015}},{{-0.01,0,0.025}},{{0.02,0.04,-0.05}},{{-0.04,-0.03,0.04}},{{0.025,0.025,0.025}},{{0.015,-0.025,-0.025}},{{0.04,-0.03,0.03}},{{0.03,-0.03,-0.04}},{{0,0,0}},{{0.02,-0.02,-0.03}},{{-0.025,-0.01,-0.015}},{{0,0,0}},{{0,0.015,0}},{{-0.015,0.015,0}},{{0.03,0.02,0.03}},{{-0.03,0,0.01}},{{0.05,-0.04,0.05}},{{-0.03,0.03,0.03}},{{-0.03,-0.03,-0.04}},{{-0.03,0.03,-0.03}},{{0.04,-0.02,0.05}},{{0.05,-0.03,0.05}},{{-0.025,0,0.01}},{{0.015,-0.01,-0.01}},{{-0.04,0.03,0.02}},{{0,0,0}},{{0,0,0.015}},{{0.015,0,-0.015}},{{-0.03,-0.03,-0.04}},{{0.04,0.03,0.02}},{{0,0,0}},{{0.01,-0.025,0.01}},{{-0.015,0.015,0.025}},{{-0.03,-0.02,-0.03}},{{-0.015,-0.01,-0.01}},{{0.04,0.02,0.04}},{{-0.03,0.02,0.03}},{{0.05,0.03,-0.04}},{{0.03,0.03,0.02}},{{0.04,-0.03,-0.04}},{{0.03,-0.02,0.04}}
                    },
                    {
                    {{-0.06,-0.04,0.04}},{{-0.03,-0.03,-0.03}},{{-0.015,-0.01,-0.01}},{{0.04,0.03,0.04}},{{0.07,0.04,-0.07}},{{-0.05,-0.03,0.03}},{{0.03,0.03,-0.05}},{{0.02,0.04,-0.07}},{{-0.04,-0.04,0.06}},{{0,0.015,0.015}},{{-0.01,0,0.025}},{{0.07,-0.03,0.03}},{{0.02,0.03,-0.05}},{{-0.04,-0.04,0.06}},{{0.06,-0.03,-0.05}},{{0,0,0}},{{0.02,-0.03,-0.03}},{{0.04,-0.04,0.03}},{{0.03,-0.03,-0.05}},{{0.02,-0.03,-0.03}},{{-0.03,0.02,0.03}},{{0.05,0.03,-0.04}},{{0.03,0.04,0.02}},{{0.04,-0.03,-0.04}},{{-0.025,-0.01,-0.015}},{{0.03,0.03,0.03}},{{0.04,0.04,0.02}},{{-0.03,0,0.01}},{{0.06,-0.04,0.06}},{{-0.04,0.04,0.04}},{{-0.06,-0.04,-0.06}},{{-0.04,0.03,-0.04}},{{-0.025,0,0.01}},{{0.015,-0.01,-0.01}},{{-0.06,0.03,0.02}},{{0.04,-0.03,0.06}},{{0.06,-0.04,0.05}},{{-0.06,0.03,0.02}},{{-0.05,-0.03,-0.05}},{{-0.03,0.02,-0.03}},{{0.04,-0.02,0.05}},{{0.05,-0.02,0.05}},{{0.05,0.04,0.02}},{{0,0,0}},{{-0.03,-0.03,-0.03}},{{-0.03,0,0.01}},{{0.05,-0.02,0.05}},{{-0.03,0.03,0.03}},{{0.04,0.02,0.04}},{{-0.03,0.01,0.03}},{{0.07,0.03,-0.05}},{{0.03,0.03,0.02}},{{0.05,-0.04,-0.06}},{{0.03,-0.02,0.06}},{{0.03,-0.02,0.05}}
                    }
                    };
                cf::createWeapon(
                    newgun
                    ,W_MACHINEGUN
                    ,false
                    ,400
                    ,400
                    ,3.5f
                    ,0.f
                    ,MODEL_WEAPON_MACHINEGUN
                    ,{1.2f,-0.8f,-2.f}
                    ,SOUND_SFX_WEAPON_MACHINEGUN
                    ,true
                    ,shootDesv
                    ,aceleratedShoot
                );
                break;
            case W_SNIPER:  
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_SNIPER, SOUNDS_VOICE);
                shootDesv={{{{0,0,0}}}};  
                cf::createWeapon(
                    newgun
                    ,W_SNIPER
                    ,false
                    ,30
                    ,100
                    ,8.f //at max charge it does to the first enemy it hits 3*base damage and the damage of the area, wich it's 0.75*damage of the shoot
                    ,0.f
                    ,MODEL_WEAPON_SNIPER
                    ,{1.f,-1.4f,-2.f}
                    ,SOUND_SFX_WEAPON_SNIPER
                    ,true
                    ,shootDesv
                    ,sniperShoot);
                break;
            case W_CARBINE:
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_SEMIAUTO, SOUNDS_VOICE);
                shootDesv={{
                        {{0,0,0}},{{0.015,0,0}},{{0,0.015,0.015}},{{-0.01,0,0.025}},{{0.025,0.025,0.025}},{{0.015,-0.025,-0.025}},{{-0.025,-0.01,-0.015}},{{0,0,0}},{{0,0.015,0}},{{-0.015,0.015,0}},{{-0.025,0,0.01}},{{0.015,-0.01,-0.01}},{{-0.015,-0.0015,-0.0015}},{{-0.01,-0.0015,-0.01}},{{0,0,0}},{{0,0,0.015}},{{0.015,0,-0.015}},{{0.01,-0.025,0.01}},{{-0.015,0.015,0.025}},{{-0.015,-0.01,-0.01}}
                }};
                cf::createWeapon(
                    newgun
                    ,W_CARBINE
                    ,true
                    ,120
                    ,400
                    ,4.5f
                    ,0.f
                    ,MODEL_WEAPON_SEMIAUTOMATIC
                    ,{1.f,-1.f,-2.f}
                    ,SOUND_SFX_WEAPON_SEMIAUTOMATIC
                    ,false
                    ,shootDesv
                    ,basicShoot
                    ,STAT_SLOWED
                    ,4.f);
                break;
            case W_SPECIAL:
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_CANNON, SOUNDS_VOICE);
                shootDesv={{{{0,0,0}}}};
                cf::createWeapon(
                    newgun
                    ,W_SPECIAL
                    ,true
                    ,7
                    ,50
                    ,50.f
                    ,0.f
                    ,MODEL_WEAPON_CANNON
                    ,{1.f,-1.0f,-1.f}
                    ,SOUND_SFX_WEAPON_CANNON
                    ,false
                    ,shootDesv
                    ,retroShoot);
                break;
            case W_EXPLOSIVE:
                if(weaponPhrase && gctx)
                     gctx->audioManager.playSound(SOUND_VFX_LIAM_PICKUP_EXPLOSIVE, SOUNDS_VOICE);
                shootDesv={{{{0,0,0}}}};
                cf::createWeapon(
                    newgun
                    ,W_EXPLOSIVE
                    ,true
                    ,17
                    ,50
                    ,25.f
                    ,0.f
                    ,MODEL_WEAPON_EXPLOSIVE
                    ,{1.5f,-0.8f,-2.f}
                    ,SOUND_SFX_WEAPON_EXPLOSIVE
                    ,false
                    ,shootDesv
                    ,explosiveShoot);
                break;

            case W_EMPTYHAND:
                shootDesv = {};
                cf::createWeapon(
                    newgun
                    ,W_EMPTYHAND
                    ,true
                    ,-2
                    ,300
                    ,3.f
                    ,0.f
                    ,"" // Empty model means no model
                    ,{1.f,-1.2f,-2.f}
                    ,SOUND_SFX_WEAPON_HANDGUN // Add sound of fist attack?
                    ,false
                    ,shootDesv
                    ,nullShoot
                );
                break;
            
            case W_ENEMY:
                shootDesv={{
                   {{0,0,0}},
                   {{1.f,0,-1.f}},
                   {{1.f,0,3.f}},
                   {{-3.f,-1.f,-2.f}},
                   {{0,0,0}},
                   {{-1.f,0,1.f}},
                   {{1.f,0,-3.f}},
                   {{3.f,-1.f,-2.f}},
                    }
                };
                cf::createWeapon(
                    newgun
                    ,W_ENEMY
                    ,true
                    ,2
                    ,50
                    ,2.f
                    ,0.f
                    ,"" // No model
                    ,{0.5f,1.2f,-0.5f}
                    ,SOUND_SFX_WEAPON_ENEMY_SHOOT
                    ,false
                    ,shootDesv
                    ,enemyShoot);
                break;
        }

        auto& weaponvec = cmp.weapons;
        weaponvec.push_back(newgun);
        std::sort(weaponvec.begin(), weaponvec.end(), compareID);

        // Throw tutorial and save weapon in save data
        if(!weaponPhrase || !gctx) return;
        showWeaponTutorial(id, *gctx);
        gctx->saveDataManager.saveObtainedWeapon(id, true);
    }

    void restoreAmmo(WeaponComponent& cmp) noexcept
    {
        auto& weaponvec = cmp.weapons;

        for(auto& weap : weaponvec)
        {
            weap.ammo += weap.maxAmmo * WEAPON_RESTORE_PER;

            if(weap.ammo>weap.maxAmmo)  
                weap.ammo = weap.maxAmmo;
        }
    }

    void equipWeapon(WeaponComponent& cmp, GameContext& gctx, WeaponID id, bool getIfNotObtained, bool sendOnline) noexcept
    {
        auto& weaponvec = cmp.weapons;
        auto iter = std::find_if(weaponvec.begin(), weaponvec.end(),
            [id](Weapon a) {
                a.deltAcumulator=a.shootingPeriod;
                return a.id==id;
            });
        
        if(iter != weaponvec.end())
        {
            // Stop weapon sound just in case is continuous
            int newIndex = iter-weaponvec.begin(); 
            if(newIndex != cmp.index)
                stopWeaponSound(cmp.weapons[cmp.index], gctx);
            
            // Change to new index
            cmp.index = newIndex;

            // Send the info to other machines that weapon was changed
            if(sendOnline)
            {
                TypeComponent* type = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
                gctx.networkClient.send(
                    "PlayerWeaponChanged" SEP
                    +std::to_string(type->idInServer)
                    +SEP+std::to_string(id)
                );
            }
        }
        // If we don't have the weapon but the "getIfNotObtained" is true
        // then obtain and equip the weapon
        else if(getIfNotObtained)
        {
            addWeapon(cmp, id);
            equipWeapon(cmp, gctx, id);
        }

        // Mark in HUD
        if(gctx.gameManager.getLocalPlayerID()==(int)cmp.getEntityID())
            gctx.hudManager.showWeaponBar(cmp);
    }

    void cycleWeapon(WeaponComponent& cmp, GameContext& gctx, int i) noexcept
    {
        auto prevIndex = cmp.index;
        cmp.index -= i;

        if(cmp.index >= (int)cmp.weapons.size()) 
            cmp.index = 0;
        else if(cmp.index < 0)
            cmp.index = cmp.weapons.size() - 1;

        // Show bar if a change was requested
        if(i != 0)
            if(gctx.gameManager.getLocalPlayerID()==(int)cmp.getEntityID())
                gctx.hudManager.showWeaponBar(cmp);

        // If weapon has changed
        if(prevIndex != cmp.index)
        {
            // Stop weapon sound just in case is continuous
            stopWeaponSound(cmp.weapons[prevIndex], gctx);

            // Mark in HUD
            auto weaponId = cmp.weapons[cmp.index].id;
            

            // Send change online
            TypeComponent* type = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
            gctx.networkClient.send(
                "PlayerWeaponChanged" SEP
                +std::to_string(type->idInServer)
                +SEP+std::to_string(weaponId)
            );
        }
    }

    /*Plays the sound of the specified weapon, including continous sound logic like the machine gun*/
    void playWeaponSound(Weapon& weapon, GameContext& gctx, const Vector3f& shooterPosition)
    {
        if(!weapon.parameterSound)
        {
            gctx.audioManager.play3DSound(weapon.sound, SOUNDS_SFX_COMBAT, shooterPosition);
        }
        else
        {
            if(gctx.audioManager.getSoundParameter(weapon.sound,"isShooting", SOUNDS_SFX_COMBAT)==0)
            {
                gctx.audioManager.setSoundParameter(weapon.sound,"isShooting",1, SOUNDS_SFX_COMBAT);
                gctx.audioManager.play3DSound(weapon.sound, SOUNDS_SFX_COMBAT, shooterPosition);
            }
        }
    }

    /*Plays the sound of no ammo*/
    void playNoAmmoSound(Weapon& weapon, GameContext& gctx, const Vector3f& shooterPosition)
    {
        gctx.audioManager.play3DSound(SOUND_SFX_WEAPON_NOAMMO, SOUNDS_SFX_COMBAT, shooterPosition);
    }


    /*Stops weapon sounds for continous sound weapons like the machinegun*/
    void stopWeaponSound(Weapon& weapon, GameContext& gctx)
    {
        weapon.shootingTime = 0.f;
        gctx.audioManager.setSoundParameter(weapon.sound,"isShooting",0, SOUNDS_SFX_COMBAT);
    }

    void createWeapon(
        Weapon &gun, 
        WeaponID id,
        bool semi,
        int maxammo,
        int rpm,
        float dmg, 
        float acc, 
        std::string 
        path,
        Vector3f offset, 
        std::string sound, 
        bool soundParam,
        RotationPackage const& shootDesv,
        WeaponFunction weaponBeh,
        StatusType stat,
        float statTime
    ) noexcept
    {
        //variable
        gun.id = id;
        gun.isSemiauto = semi;
        gun.maxAmmo = maxammo;
        gun.fireRate = rpm;
        gun.damage = dmg;
        gun.deviationFactor = acc;
        gun.meshFilepath = path;
        gun.offset = offset;
        gun.sound = sound;
        gun.parameterSound = soundParam;
        gun.shootsDeviation = shootDesv;
        gun.weaponBehaviour= weaponBeh;
        gun.status= stat;
        gun.statusTime=statTime;

        //non-variable
        gun.isEquipped =            true;
        gun.isFiring =              false;
        gun.flagFiredShot =         false;
        gun.ammo =                  gun.maxAmmo;
        gun.shootingPeriod =        60.f/(float)gun.fireRate;
        gun.deltAcumulator =        gun.shootingPeriod;
        gun.shootingTime =          0;
        gun.deviationSelector=      0;
    }

    void showWeaponTutorial(WeaponID weaponID, GameContext& gctx)
    {
        std::string sentence {""};
        std::string icontex  {""};
        switch (weaponID)
        {
            case W_HANDGUN:
                sentence = "Fire with left mouse button\nIt's not very powerful but\nit does the job!";
                icontex  = "weaponIconHandgun";
                break;
            case W_SHOTGUN:
                sentence = "Shotgun: Fires Dragon's \nbreath pellets that sets\nyour enemies on fire!";
                icontex  = "weaponIconShotgun";
                break;
            case W_CARBINE:
                sentence = "Carbine: Shocks your\nenemies and slows them\ndown!";
                icontex  = "weaponIconCarbine";
                break;
            case W_MACHINEGUN:
                sentence = "Machinegun: Hold fire to\nincrease fire rate!";
                icontex  = "weaponIconMachinegun";
                break;
            case W_SNIPER:
                sentence = "Sniper: Hold the trigger\nto charge a piercing\nshot";
                icontex  = "weaponIconSniper";
                break;
            case W_EXPLOSIVE:
                sentence = "Rocket Launcher:\nIt launches rockets";
                icontex  = "weaponIconRocketLnch";
                break;
            case W_SPECIAL:
                sentence = "Retroman Cannon: Unlimited\npower! But not so\nunlimited ammo...";
                icontex  = "weaponIconRetroman";
                break;
            default: break;
        }
        gctx.hudManager.addPopup(
            "topSlot"
            , "weaponTutorial"
            , sentence
            , "popupBackgroundL"
            , icontex
            , 5.f
        );
    }

}

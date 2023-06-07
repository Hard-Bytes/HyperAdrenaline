#pragma once
#include "weaponCmp.hpp"
#include "../context/gameContext.hpp"

// cf = Component Functions
namespace cf
{
    void addWeapon(WeaponComponent& cmp, WeaponID id, GameContext* gc=nullptr, bool weaponPhrase=false) noexcept;
    void restoreAmmo(WeaponComponent& cmp) noexcept;
    void equipWeapon(WeaponComponent& cmp, GameContext& gc, WeaponID id, bool getIfNotObtained=false, bool sendOnline=true) noexcept;
    void cycleWeapon(WeaponComponent& cmp, GameContext& gc, int i) noexcept;
    void playWeaponSound(Weapon& weapon, GameContext& gc, const Vector3f& shooterPosition);
    void playNoAmmoSound(Weapon& weapon, GameContext& gc, const Vector3f& shooterPosition);
    void stopWeaponSound(Weapon& weapon, GameContext& gc);
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
        bool sounParam,
        RotationPackage const& deviation,
        WeaponFunction weaponBeh,
        StatusType stat=STAT_NORMAL,
        float statusTime=2.5
    ) noexcept;
    void showWeaponTutorial(WeaponID weaponID, GameContext& gc);
}

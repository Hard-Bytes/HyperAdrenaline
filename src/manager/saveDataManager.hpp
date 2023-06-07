#pragma once
#include <vector>

struct SaveDataManager
{
    void saveObtainedWeapon(int weapon, bool obtained);
    std::vector<int> loadObtainedWeapons();

    static inline std::vector<const char*> weaponNames {
        "handgun"
        ,"shotgun"
        ,"carbine"
        ,"machinegun"
        ,"sniper"
        ,"explosive"
        ,"retroman"
    };
};

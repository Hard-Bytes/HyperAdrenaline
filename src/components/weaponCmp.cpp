#include "weaponCmp.hpp"
#include "weaponCmpFunc.hpp"

WeaponComponent::WeaponComponent(EntityID entID, Vector3f offset)
    : Component(entID), generalOffset(offset)
{
    cf::addWeapon(*this, W_EMPTYHAND);
}

WeaponComponent::~WeaponComponent()
{
    /*
    for(auto& weapon : weapons)
    {
        if(weapon.gunNode)
        {
            auto* gunNode = weapon.gunNode;
            delete gunNode;
            weapon.gunNode = nullptr;
        }
    }
    */
}

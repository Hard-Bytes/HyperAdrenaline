#include "weapon.hpp"
#include "../components/healthCmpFunc.hpp"
#include "../components/weaponCmpFunc.hpp"


WeaponSystem::WeaponSystem(GameContext& gc)
    : gctx(gc)
{}

WeaponSystem::~WeaponSystem()
{}

void WeaponSystem::updateOne(WeaponComponent& cmp) const noexcept
{
    if(cmp.weapons.empty()) return;

    NodeComponent* parentNode = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    for(auto& w : cmp.weapons)
    {
        if(w.meshFilepath == "") continue; // Skip repositioning if it has no model (fist, for example)
        if(!w.gunNode)
        {
            w.gunNode = gctx.graphicsEngine.createNode(
                cmp.generalOffset + w.offset, // Position is assigned down below
                cmp.generalScale,
                w.meshFilepath
            );
            parentNode->node->appendChild(w.gunNode);
            w.gunNode->setScale(cmp.generalScale / parentNode->node->getScale());
            w.gunNode->setRotation({0,180,0});
        }
        w.gunNode->setVisible(false);
    }

    Weapon& weapon = cmp.weapons[cmp.index];

    weapon.isEquipped = true;

    if(weapon.gunNode)
    {
        weapon.gunNode->setVisible(true);
        //the position RELATIVE to the father: wherever the father is, it's {0,0,0}
        weapon.gunNode->setPosition(cmp.generalOffset + weapon.offset);
    }

    // Implementada lógica de full auto y semi auto
    // en el input la variable de isFiring se actualiza
    // según si la función es isKeyDown o getSinglePress
    // Cambiar eso para que sean behaviours específicos
    //weapon.flagFiredShot = false;
    
    if(!weapon.isEquipped)  return;

    weapon.weaponBehaviour(weapon, cmp ,gctx, parentNode->node->getPosition());
    
}

void WeaponSystem::updateAll() const noexcept
{
    auto& components = gctx.entityManager.getComponentVector<WeaponComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}
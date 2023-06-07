#pragma once
#include "../context/gameContext.hpp"

//temporal include, not the fanciest but it works 
// Replacement del bulletsystem, pues no hay bullets xd
// trata el comportamiento de las armas
class WeaponSystem
{
private:
    GameContext& gctx;
public:
    explicit WeaponSystem(GameContext& gc);
    ~WeaponSystem();

    void updateOne(WeaponComponent& cmp) const noexcept;
    void updateAll() const noexcept;
};



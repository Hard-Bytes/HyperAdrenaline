#pragma once
#include <algorithm>
#include <iostream>
#include "component.hpp"
#include "behaviourFunctions.hpp"
#include "statusCmp.hpp"



#define WEAPON_RESTORE_PER 0.3f

enum WeaponFiremode 
{
    W_SEMI_AUTO_PRESS = 0,
    W_SEMI_AUTO_RELEASE,
    W_FULL_AUTO
};

// En un futuro se pueden usar templates 
// o cargar los datos del arma de un fichero
// de momento es un switch pocho
enum WeaponID
{
    W_HANDGUN = 1,
    W_SHOTGUN,
    W_CARBINE,
    W_MACHINEGUN,
    W_SNIPER,
    W_EXPLOSIVE,
    W_SPECIAL,
    W_EMPTYHAND,
    W_ENEMY
};

//1 Vector3f                                        : Coordenadas de desviacion del disparo
//2 std::vector<Vector3f>                           : Conjunto de disparos a la vez
//3 std::vector<std::vector<Vector3f>>              : conjunto de disparos que van rotando en un conjunto mas grande
//4 std::vector<std::vector<std::vector<Vector3f>>> : Diferentes conjuntos de rotacion de disparos
using ShootingPackage = std::vector<std::vector<Vector3f>>;
using RotationPackage = std::vector<ShootingPackage>;

struct Weapon {
    WeaponID id;
    bool isEquipped, isFiring, isSemiauto, flagFiredShot, parameterSound;
    bool prevIsFiring; // Used to send the message online only if this changed
    int fireRate, ammo, maxAmmo;
    float shootingPeriod, deviationFactor, damage;
    //time Floats
    float shootingTime , deltAcumulator;
    std::string sound;
    Vector3f offset;
    std::string meshFilepath;
    GraphicNode* gunNode {nullptr};

    StatusType status;
    float statusTime;
    // Action to do upon being shooting
    WeaponFunction weaponBehaviour {nullptr};

    RotationPackage shootsDeviation;
    
    int deviationSelector;
};
    bool compareID(Weapon&, Weapon&);

struct WeaponComponent : public Component
{
    // Constructor
    explicit WeaponComponent(EntityID entID, Vector3f offset={0,0,0});
    ~WeaponComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeWeapon; }

    // Default values
    std::vector<Weapon> weapons;
    int index{0}; //TODO

    Vector3f generalOffset{0,0,0};
    Vector3f generalScale{1,1,1};
};
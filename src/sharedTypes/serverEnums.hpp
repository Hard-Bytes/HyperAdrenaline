#pragma once

#define ATOI(n) atoi(msg[n].c_str())
#define ATOF(n) atof(msg[n].c_str())

#define ONLINE_SEPARATOR "&"

enum PlayerLook {
    PL_Liam
    ,PL_Captain
};

enum DoorType {
    D_InteractableDoor
    ,D_CreditsDoor
    ,D_KeyDoor
    ,D_RemoteDoor
    ,D_KillsDoor
};

enum PickupType {
    PK_Key
    ,PK_Weapon
    ,PK_Medkit
    ,PK_Ammo
};

enum MapElementType {
    ME_Button = 0
    ,ME_ButtonKeyControlled
    ,ME_MovingPlatform
    ,ME_MovingPlatformRemote
    ,ME_MedkitBase
    ,ME_AmmoBase
    ,ME_MedkitVendingMachine
    ,ME_AmmoVendingMachine
    ,ME_Solid
    ,ME_KillingArea
    ,ME_DamagingArea
    ,ME_LevelExit
};

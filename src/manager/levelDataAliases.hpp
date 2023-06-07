#pragma once

// Helpful macros
#define GET_VEC3(vec) {vec[0].GetFloat(), vec[1].GetFloat(), vec[2].GetFloat()}

// Defines for json properties
#define MAP_SECTION_STRUCTURE "mapStructure"
    #define MAP_DATA_MAPNAME        "mapName"
    #define MAP_DATA_MESHFILE       "meshFile"
    #define MAP_DATA_NAVMESHFILE    "navmeshFile"
    #define MAP_DATA_BGMUSIC        "bgMusic"
    #define MAP_DATA_PLAYERSPAWN    "playerSpawnPoint"
    #define MAP_DATA_PLAYERLOOKAT   "playerLookAt"
    #define MAP_DATA_MINYCOORD      "minYCoordinate"
    #define MAP_DATA_WAYPOINTS      "waypoints"
    #define MAP_DATA_ROUTES         "routes"

#define MAP_SECTION_GENERAL_DATA "generalData"
    #define MAP_DATA_RAID_TIME      "raidTime"
    #define MAP_DATA_ENEMIESINRAID  "enemiesInARaid"
    #define MAP_DATA_ENEMYSPAWNPTS  "initialEnemySpawnPoints"

#define MAP_SECTION_ELEMENTS "mapElements"
    #define MAP_DATA_LEVEL_EXIT     "levelExits"
    #define MAP_DATA_ELEM_KILLDOORS "enemyKillDoors"
    #define MAP_DATA_ELEM_KEYDOORS  "keyDoors"
    #define MAP_DATA_ELEM_KEYS      "keys"
    #define MAP_DATA_PADS           "pads"
    #define MAP_DATA_VENDING_MACH   "vendingMachines"
    #define MAP_DATA_PICKUP_WEAPON  "pickableWeapon"
    #define MAP_DATA_ENEMIES        "prespawnedEnemies"
    #define MAP_DATA_LIGHTS         "lights"
    #define MAP_DATA_ROOMSENSORS    "roomSensors"
    #define MAP_DATA_INVISIBLE_WALLS "invisibleWalls"
    

// Defines for specific data structures
#define LEVELEXIT_POSITION          "pos"
#define LEVELEXIT_ROTATION          "rot"
#define LEVELEXIT_SCALE             "scale"

#define KILLDOOR_POSITION           "pos"
#define KILLDOOR_ROTATION           "rot"
#define KILLDOOR_SCALE              "scale"
#define KILLDOOR_KILLS_REQUIRED     "killsRequired"
#define KILLDOOR_RESPAWN_TIME       "respawnTime"
#define KILLDOOR_SPAWNPOINTS        "spawnPoints"
#define KILLDOOR_RAID_ZONE          "raidZone"

#define KEYDOOR_POSITION            "pos"
#define KEYDOOR_ROTATION            "rot"
#define KEYDOOR_SCALE               "scale"
#define KEYDOOR_KEY_ID              "keyID"
#define KEYDOOR_REMOTE_ID           "remoteID"
#define KEYDOOR_SPAWNPOINTS         "spawnPoints"
#define KEYDOOR_RAID_ZONE           "raidZone"
#define KEYDOOR_RESPAWN_TIME        "respawnTime"

#define KEY_POSITION                "pos"
#define KEY_SCALE                   "scale"
#define KEY_PARAM_ID                "keyID"

#define PAD_POSITION                "pos"
#define PAD_SCALE                   "scale"
#define PAD_TYPE                    "padType"
    #define PAD_TYPE_HEALTH 1
    #define PAD_TYPE_AMMO   2

#define VENDINGMACH_POSITION        "pos"
#define VENDINGMACH_SCALE           "scale"
#define VENDINGMACH_ROTATION        "rot"
#define VENDINGMACH_TYPE            "vendingMachineType"
    #define VENDINGMACH_TYPE_HEALTH 1
    #define VENDINGMACH_TYPE_AMMO   2
#define VENDINGMACH_PRICE            "vendingMachinePrice"

#define PICKWEAPON_POSITION         "pos"
#define PICKWEAPON_SCALE            "scale"
#define PICKWEAPON_TYPE             "weaponType"

#define PRESPAWNEDENEMY_TYPE        "enemyType"
#define PRESPAWNEDENEMY_POSITION    "pos"
#define PRESPAWNEDENEMY_SCALE       "scale"
#define PRESPAWNEDENEMY_PATROL      "patrol"
#define PRESPAWNEDENEMY_INIT_PT     "initialPatrolPoint"
#define PRESPAWNEDENEMY_ISRAID      "isFromRaid"
#define PRESPAWNEDENEMY_ROTATION    "rotation"

#define LIGHT_POSITION              "pos"
#define LIGHT_TYPE                  "type"
#define LIGHT_RADIUS                "radius"
#define LIGHT_INTENSITY_AMB         "intensityAmbient"
#define LIGHT_INTENSITY_DIF         "intensityDiffuse"
#define LIGHT_INTENSITY_SPE         "intensitySpecular"
#define LIGHT_ATTEN_CONST           "attenuationConstant"
#define LIGHT_ATTEN_LINEAR          "attenuationLinear"
#define LIGHT_ATTEN_QUADRA          "attenuationQuadratic"
#define LIGHT_DIRECTION             "direction"
#define LIGHT_INNERCUTOFF           "innerCutoff"
#define LIGHT_OUTERCUTOFF           "outerCutoff"

#define SENSOR_CORNERFROM           "cornerFrom"
#define SENSOR_CORNERTO             "cornerTo"

#define INVISIBLE_WALL_POSITION     "pos"
#define INVISIBLE_WALL_SIZE         "size"

#define SPAWNPOINT_POSITION         "pos"
#define SPAWNPOINT_PATROL           "patrol"
#define SPAWNPOINT_INIT_PATROL_PT   "initialPatrolPoint"
#define SPAWNPOINT_ENEMY_TYPE       "enemyType"
#define SPAWNPOINT_IS_FOR_RAID      "isForRaid"
#define SPAWNPOINT_OPT_NUM_ENEMIES  "numEnemiesToSpawn"

#pragma once
#include <string>
#include <list>
#include <sharedTypes/serverEnums.hpp>
#include <websocketpp/common/connection_hdl.hpp>

using ValueList = std::list<float>;

struct Vec3 {
    float x{0}, y{0}, z{0};
    Vec3():x(0),y(0),z(0){}
    Vec3(float px, float py, float pz):x(px),y(py),z(pz){}
};

struct PlayerData {
    std::string name;
    Vec3 position;
    Vec3 rotation;
    websocketpp::connection_hdl hdl; // To associate a player to its connection
    float health;
    int equippedWeapon{0};
    int credits{0};
    int look;
    int id { -1 };
    bool isHost {false};
    // max health (define if max life can change)

    void resetEntity()
    {
        this->id = -1;
        this->position = {0,0,0};
        this->rotation = {0,0,0};
        this->health = 0;
        this->equippedWeapon = 0;
        this->credits = 0;
    }
};

struct EnemyData {
    Vec3 position;
    Vec3 scale;
    Vec3 rotation;
    float health;
    int enemyType;
    int id;
    bool isFromRaid;
};

struct DoorData {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    DoorType doorType;
    int id;
    bool isOpen;
    // possible
    int remoteID{-1};
    int creditCost{0};
};

struct PickupData {
    Vec3 position;
    Vec3 rotation;
    int id;
    PickupType pickupType;
    ValueList dataValues; // possible multivalues
};

struct MapElementData {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    MapElementType elementType;
    int sharedKeyId{0};
    int idRemoteToInteractWith{-1};
    int idRemoteToBeInteracted{-1};
    float interactionData{0.0f};
    int id;
};

struct RoundData {
    float timeForNextRound;
    int currentRound;
    bool isActive;
};

using PlayerList = std::list<PlayerData>;
using EnemyList = std::list<EnemyData>;
using DoorList = std::list<DoorData>;
using PickupList = std::list<PickupData>;
using MapElementList = std::list<MapElementData>;

struct GameStatus {
    PlayerList playerList;
    EnemyList enemyList;
    DoorList doorList;
    PickupList pickupList;
    MapElementList mapElementList;

    RoundData round;
    int enemyKillCount;
    int mapIndex {-1};

    unsigned int nextId{0};
};

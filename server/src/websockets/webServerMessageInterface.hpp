#pragma once
#include "wsServerMessages.hpp"
#include <unordered_map>

struct broadcast_server;

class WebServerMessageInterface
{
public:
    // Constructor
    explicit WebServerMessageInterface();
    ~WebServerMessageInterface();

    void processMessage(broadcast_server* s, websocketpp::connection_hdl hdl, std::string msg);
private:
    GameStatus status;

    using MsgMap = std::unordered_map<std::string, ServerMessageProcessing>; 
    MsgMap messageProcessingFunctions {
        { "ClientConnected" , wsClientConnected },
        { "ClientDisconnected" , wsClientDisconnected },

        { "MapLoaded" , wsMapLoaded },
        { "MapReady" , wsMapReady },

        { "PlayerSpawned" , wsPlayerSpawned },
        { "EnemySpawned" , wsEnemySpawned },
        { "DoorSpawned" , wsDoorSpawned },
        { "PickupSpawned" , wsPickupSpawned },
        { "MapElementSpawned" , wsMapElementSpawned },

        { "PlayerWeaponChanged" , wsPlayerWeaponChanged },
        { "PlayerPointsChanged" , wsPlayerPointsChanged },
        { "EntityMoved" , wsEntityMoved },
        { "EntityAnimationChanged" , wsEntityAnimationChanged },
        { "EntityHealthChanged" , wsEntityHealthChanged },

        { "StatusApplied" , wsStatusApplied },

        { "PlayerJumped" , wsPlayerJumped },
        { "PlayerDashed" , wsPlayerDashed },
        { "PlayerIsShooting" , wsPlayerIsShooting },
        { "EnemyBulletShooted" , wsEnemyBulletShooted },

        { "DoorOpened" , wsDoorOpened },
        { "PickupCollected" , wsPickupCollected },
        { "ButtonInteracted" , wsButtonInteracted },
        { "VendingMachineActivated" , wsVendingMachineActivated },

        { "EnemiesKilledCountChanged" , wsEnemiesKilledCountChanged },
        { "RoundChanged" , wsRoundChanged },
        { "RoundFinished" , wsRoundFinished },

        { "LevelFinished" , wsLevelFinished },

        { "GetLevel" , wsGetLevel },
        { "GetWorldStatus" , wsGetWorldStatus },

        { "PlayerConnected" , wsPlayerConnected },
        { "PlayerDisconnected" , wsPlayerDisconnected },
        { "SelectionChanged" , wsSelectionChanged },
        { "LevelSelected" , wsLevelSelected },
        { "GetPlayerList" , wsGetPlayerList }
    };
};

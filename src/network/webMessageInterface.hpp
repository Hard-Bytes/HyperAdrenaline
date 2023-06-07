#pragma once
#include "wsClientMessages.hpp"
#include <unordered_map>
#include <queue>

class GameContext;

class WebMessageInterface
{
public:
    // Constructor
    explicit WebMessageInterface();
    ~WebMessageInterface();

    void queueMessage(std::string msg) noexcept;
    void processAllMessages() noexcept;
    void setGameContext(GameContext*) noexcept;

    constexpr void setAllowEntitySpawning(bool allow) noexcept
        { m_allowEntitySpawning = allow; }
    constexpr bool getAllowEntitySpawning(void) const noexcept
        { return m_allowEntitySpawning; }

    void setDisconnectionReason(const std::string& reason) noexcept
        { m_disconnectionReason = reason; }
    const std::string&  getDisconnectionReason(void) const noexcept
        { return m_disconnectionReason; }
    void resetDisconnectionReason() noexcept
        { m_disconnectionReason = "Server is not online"; }
private:
    void processMessage(std::string msg) noexcept;
    
    bool m_allowEntitySpawning {false};
    std::string m_disconnectionReason {"Server is not online"};

    GameContext* gctx;

    std::queue<std::string> messageQueue;

    using MsgMap = std::unordered_map<std::string, ClientMessageProcessing>; 
    MsgMap messageProcessingFunctions {
        { "VinculateEntity" , wsVinculateEntity },
    
        { "MapLoaded" , wsMapLoaded },

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
        
        { "ForceDisconnect" , wsForceDisconnect },

        { "PlayerConnected" , wsPlayerConnected },
        { "PlayerDisconnected" , wsPlayerDisconnected },
        { "SelectionChanged" , wsSelectionChanged },
        { "LevelSelected" , wsLevelSelected }
    };
};

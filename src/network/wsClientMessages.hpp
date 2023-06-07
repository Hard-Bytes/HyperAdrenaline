#pragma once
#include <string>
#include <vector>

class GameContext;

using StrVec = std::vector<std::string>;

#define OnlineClientFunctionParams GameContext* gctx, const StrVec& msg
#define declareClientMsgResponse(func) void func(OnlineClientFunctionParams)

// Behaviour Functions
typedef void (*ClientMessageProcessing)
    (OnlineClientFunctionParams);

// Game messages
declareClientMsgResponse(wsVinculateEntity);
declareClientMsgResponse(wsMapLoaded);

declareClientMsgResponse(wsPlayerSpawned);
declareClientMsgResponse(wsEnemySpawned);
declareClientMsgResponse(wsDoorSpawned);
declareClientMsgResponse(wsPickupSpawned);
declareClientMsgResponse(wsMapElementSpawned);

declareClientMsgResponse(wsPlayerWeaponChanged);
declareClientMsgResponse(wsPlayerPointsChanged);
declareClientMsgResponse(wsEntityMoved);
declareClientMsgResponse(wsEntityAnimationChanged);
declareClientMsgResponse(wsEntityHealthChanged);

declareClientMsgResponse(wsStatusApplied);

declareClientMsgResponse(wsPlayerJumped);
declareClientMsgResponse(wsPlayerDashed);
declareClientMsgResponse(wsPlayerIsShooting);
declareClientMsgResponse(wsEnemyBulletShooted);

declareClientMsgResponse(wsDoorOpened);
declareClientMsgResponse(wsPickupCollected);
declareClientMsgResponse(wsButtonInteracted);
declareClientMsgResponse(wsVendingMachineActivated);

declareClientMsgResponse(wsEnemiesKilledCountChanged);
declareClientMsgResponse(wsRoundChanged);
declareClientMsgResponse(wsRoundFinished);

declareClientMsgResponse(wsLevelFinished);

declareClientMsgResponse(wsForceDisconnect);

// Lobby messages
declareClientMsgResponse(wsPlayerConnected);
declareClientMsgResponse(wsPlayerDisconnected);
declareClientMsgResponse(wsSelectionChanged);
declareClientMsgResponse(wsLevelSelected);

int getLocalIdFromServerId(int idInServer, GameContext* gctx);
StrVec parseMessage(std::string msg);

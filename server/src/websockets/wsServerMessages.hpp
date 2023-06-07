#pragma once
#include <string>
#include <vector>
#include <websocketpp/common/connection_hdl.hpp>
#include "gameStatus.hpp"

struct broadcast_server;

using StrVec = std::vector<std::string>;

#define OnlineFunctionParams broadcast_server* s, websocketpp::connection_hdl hdl, StrVec& msg, GameStatus& status
#define declareServerMsgResponse(func) void func(OnlineFunctionParams)

// Behaviour Functions
typedef void (*ServerMessageProcessing)
    (OnlineFunctionParams);

declareServerMsgResponse(wsClientConnected);
declareServerMsgResponse(wsClientDisconnected);

declareServerMsgResponse(wsMapLoaded);
declareServerMsgResponse(wsMapReady);

declareServerMsgResponse(wsPlayerSpawned);
declareServerMsgResponse(wsEnemySpawned);
declareServerMsgResponse(wsDoorSpawned);
declareServerMsgResponse(wsPickupSpawned);
declareServerMsgResponse(wsMapElementSpawned);

declareServerMsgResponse(wsPlayerWeaponChanged);
declareServerMsgResponse(wsPlayerPointsChanged);
declareServerMsgResponse(wsEntityMoved);
declareServerMsgResponse(wsEntityAnimationChanged);
declareServerMsgResponse(wsEntityHealthChanged);

declareServerMsgResponse(wsStatusApplied);

declareServerMsgResponse(wsPlayerJumped);
declareServerMsgResponse(wsPlayerDashed);
declareServerMsgResponse(wsPlayerIsShooting);
declareServerMsgResponse(wsEnemyBulletShooted);

declareServerMsgResponse(wsDoorOpened);
declareServerMsgResponse(wsPickupCollected);
declareServerMsgResponse(wsButtonInteracted);
declareServerMsgResponse(wsVendingMachineActivated);

declareServerMsgResponse(wsEnemiesKilledCountChanged);
declareServerMsgResponse(wsRoundChanged);
declareServerMsgResponse(wsRoundFinished);

declareServerMsgResponse(wsLevelFinished);

declareServerMsgResponse(wsGetLevel);
declareServerMsgResponse(wsGetWorldStatus);

// Lobby messages
declareServerMsgResponse(wsPlayerConnected);
declareServerMsgResponse(wsPlayerDisconnected);
declareServerMsgResponse(wsSelectionChanged);
declareServerMsgResponse(wsLevelSelected);
declareServerMsgResponse(wsGetPlayerList);


StrVec parseMessage(std::string msg);
std::string reassembleMessage(const StrVec& v);

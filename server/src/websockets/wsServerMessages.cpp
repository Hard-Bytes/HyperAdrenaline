#include "wsServerMessages.hpp"
#include "websocketServer.hpp"
#include "../util/macros.hpp"

#define SEP ONLINE_SEPARATOR

/*Client Connected*/
void wsClientConnected(OnlineFunctionParams)
{
    // Stuff when a client gets connected
}

/*Client Disconnected*/
void wsClientDisconnected(OnlineFunctionParams)
{
    // Prepare default data
    std::string name { "" };
    int playerID { -1 };
    bool itWasTheHost { false };

    // Remove the player associated to this connection
    auto& playerList = status.playerList;
    auto it = std::find_if(playerList.begin(), playerList.end(), [hdl](PlayerData& data) { 
        return hdl.lock() == data.hdl.lock(); 
    });
    // If player didn't exist, do nothing else
    if(it == playerList.end()) return;

    // Recover player data
    name = it->name;
    playerID = it->id;
    itWasTheHost = it->isHost;
    playerList.erase(it);

    // If it was the host, disconnect everyone else
    if(itWasTheHost)
        for(auto& player : playerList)
            s->send(player.hdl, "ForceDisconnect" SEP "Host disconnected");

    // Send disconnection to rest of players
    s->broadcast_all("PlayerDisconnected" SEP + name + SEP + std::to_string(playerID));
}


/*Map Loaded*/
// The host loaded a map, so reset the world status for new map load
// Also cleans the current game state
void wsMapLoaded(OnlineFunctionParams)
{
    // Minimum args check
    if(msg.size()<2) return;

    // Save new map index
    status.mapIndex = ATOI(1);

    // Reset world status
    status.nextId = 0;
    status.enemyList.clear();
    status.doorList.clear();
    status.pickupList.clear();
    status.mapElementList.clear();
    status.enemyKillCount = 0;
    status.round.currentRound = 1;
    status.round.isActive = false;
    status.round.timeForNextRound = 10;

    // Reset players to non-entity state
    auto& playerList = status.playerList;
    for(auto& player : playerList)
        player.resetEntity();
}

/*Map Loaded*/
// The host finsihed loading the map, tell everyone to load it too
void wsMapReady(OnlineFunctionParams)
{
    // Broadcast the new map to others
    s->broadcast_all_others(hdl, "MapLoaded" SEP+std::to_string(status.mapIndex));
}


/*Player Spawned*/
void wsPlayerSpawned(OnlineFunctionParams)
{
    // Args: messageString, localId, Vec3 pos, string name, float health, weapon(int), int skin
    if(msg.size()<7) 
        return;

    // Base elements
    // int localDoorID = ATOI(1);
    Vec3 pos(ATOF(2), ATOF(3), ATOF(4));
    float currentHealth = ATOF(6);
    int equippedWeapon = ATOI(7);

    auto& playerList = status.playerList;
    auto it = std::find_if(playerList.begin(), playerList.end(), [hdl](PlayerData& data){
        return hdl.lock() == data.hdl.lock(); 
    });
    if(it == playerList.end()) 
    {
        LOG_ERR("Player "<< msg[5] << " tried to spawn but is not connected");
        return;
    }

    PlayerData& data = *it;
    data.position = pos;
    data.health = currentHealth;
    data.equippedWeapon = equippedWeapon;

    // Assign id for server to data
    data.id = status.nextId++;

    // Tell the client that spawned this to assign the id
    std::string idInServer = std::to_string(data.id);
    s->send(hdl,
        "VinculateEntity" SEP
        +msg[1] // local id
        +SEP+idInServer
    );

    // Tell the other clients to spawn this element
    std::string result = "PlayerSpawned" SEP+idInServer;
    for(unsigned int i=2; i<msg.size(); i++)
        result += SEP+msg[i];
    
    s->broadcast_all_others(hdl, result);
}

/*Enemy Spawned*/
void wsEnemySpawned(OnlineFunctionParams)
{
    // Minimum args check
    if(msg.size()<7) return;

    // Create enemy data with message data
    //int localId = ATOI(1);
    int type = ATOI(2);
    float currentHealth = ATOF(3);
    bool isFromRaid = ATOI(4);
    Vec3 pos(ATOF(5),ATOF(6),ATOF(7));
    EnemyData data;
    data.enemyType = type;
    data.position = pos;
    data.health = currentHealth;
    data.isFromRaid = isFromRaid;

    if(msg.size()>=11)
    {
        Vec3 scale(ATOF(8),ATOF(9),ATOF(10));
        data.scale = scale;
    }
    if(msg.size()>=14)
    {
        Vec3 rot(ATOF(11),ATOF(12),ATOF(13));
        data.rotation = rot;
    }

    // Assign id for server to data
    data.id = status.nextId++;

    // Add new data to list
    status.enemyList.push_back(data);

    // Tell the client that spawned this to assign the id
    std::string idInServer = std::to_string(data.id);
    s->send(hdl,
        "VinculateEntity" SEP
        +msg[1] // local id
        +SEP+idInServer
    );

    // Tell the other clients to spawn this element
    std::string result = "EnemySpawned" SEP+idInServer;
    for(unsigned int i=2; i<msg.size(); i++)
        result += SEP+msg[i];
    
    s->broadcast_all_others(hdl, result);
}

/*Door Spawned*/
void wsDoorSpawned(OnlineFunctionParams)
{
    // Args: messageString, id, pos, rot, scale, doorType [12] optional int, int [14]
    if(msg.size()<12) 
        return;

    // Base elements
    // int localDoorID = ATOI(1);
    Vec3 pos(ATOF(2), ATOF(3), ATOF(4));
    Vec3 rot(ATOF(5), ATOF(6), ATOF(7));
    Vec3 scale(ATOF(8), ATOF(9), ATOF(10));
    int doorType = ATOI(11);

    DoorData doorDataMessage;
    doorDataMessage.position = pos;
    doorDataMessage.rotation = rot;
    doorDataMessage.scale = scale;
    doorDataMessage.doorType = (DoorType)doorType;
    doorDataMessage.id = status.nextId++;
    doorDataMessage.isOpen = false;

    // If we send the 12th, 13th arg is because it will use it
    if(msg.size()>12)
        doorDataMessage.creditCost = ATOI(12);

    if(msg.size()>13)
        doorDataMessage.remoteID = ATOI(13);
    
    /*** Send ***/
    status.doorList.push_back(doorDataMessage);

    // Tell the client that spawned this to assign the id
    std::string idInServer = std::to_string(doorDataMessage.id);
    s->send(
        hdl,
        "VinculateEntity" SEP
        +msg[1] // local id
        +SEP+idInServer
    );

    // Broadcast the message all over
    std::string result = "DoorSpawned" SEP + idInServer;

    for(unsigned int i=2; i<msg.size(); i++)
        result += SEP+msg[i];

    s->broadcast_all_others(hdl, result);
}

/*Pickup Spawned*/
void wsPickupSpawned(OnlineFunctionParams)
{
    // Args :: int id, vec3 pos, vec3 rot, enum type, [2 optional floats]
    if(msg.size()<9) 
        return;

    /*** Recover data ***/
    // int pickupID = ATOI(1);
    Vec3 pos(ATOF(2), ATOF(3), ATOF(4));
    Vec3 rot(ATOF(5), ATOF(6), ATOF(7));
    int pickupType = ATOI(8);

    /*** Prepare message ***/
    PickupData pickupDataMessage;

    pickupDataMessage.id = status.nextId++;
    pickupDataMessage.position = pos;
    pickupDataMessage.rotation = rot;
    pickupDataMessage.pickupType = (PickupType)pickupType;

    for(uint32_t i = 9; i < msg.size() ; ++i)    // Warning, type conversion might crash
        pickupDataMessage.dataValues.push_back(ATOF(i));

    // Add new data to list
    status.pickupList.push_back(pickupDataMessage);

    /*** Sending ***/
    std::string idInServer = std::to_string(pickupDataMessage.id);

    // Reassign id from server assignment over to the host
    s->send(
        hdl,
        "VinculateEntity" SEP
        +msg[1] // local id
        +SEP+idInServer
    );

    // Broadcast the message all over
    std::string result = "PickupSpawned" SEP + idInServer;
    for(unsigned int i=2; i<msg.size(); i++)
        result += SEP+msg[i];
    
    s->broadcast_all_others(hdl, result);
}

/*Map Element Spawned*/
void wsMapElementSpawned(OnlineFunctionParams)
{
    // Args: messageString, id, pos, scale, rot, enum type [12], [3 ints y 1 float opcionales] 
    if(msg.size()<12) 
        return;

    /*** Recover data ***/
    // int mapElemID = ATOI(1);
    Vec3 pos(ATOF(2), ATOF(3), ATOF(4));
    Vec3 scale(ATOF(5), ATOF(6), ATOF(7));
    Vec3 rot(ATOF(8), ATOF(9), ATOF(10));
    int mapElementType = ATOI(11);

    MapElementData mapElementDataMessage;
    mapElementDataMessage.id = status.nextId++;
    mapElementDataMessage.position = pos;
    mapElementDataMessage.rotation = rot;
    mapElementDataMessage.scale = scale;
    mapElementDataMessage.elementType = (MapElementType)mapElementType;

    if(msg.size()>12)
        mapElementDataMessage.sharedKeyId = ATOI(12);
    if(msg.size()>13)
        mapElementDataMessage.idRemoteToInteractWith = ATOI(13);
    if(msg.size()>14)  
        mapElementDataMessage.idRemoteToBeInteracted = ATOI(14);
    if(msg.size()>15)
        mapElementDataMessage.interactionData = ATOF(15);

    // Add new data to list
    status.mapElementList.push_back(mapElementDataMessage);

    /*** Sending ***/
    std::string idInServer = std::to_string(mapElementDataMessage.id);

    // Reassign id from server assignment over to the host
    s->send(
        hdl,
        "VinculateEntity" SEP
        +msg[1] // local id
        +SEP+idInServer
    );

    // Broadcast the message all over
    std::string result = "MapElementSpawned" SEP + idInServer;
    for(unsigned int i=2; i<msg.size(); i++)
        result += SEP+msg[i];
    
    s->broadcast_all_others(hdl, result);
}


/*Player Weapon Changed*/
void wsPlayerWeaponChanged(OnlineFunctionParams)
{
    // Args: messageString, server's playerID, weaponType
    if(msg.size()<3) return;

    // Go through all playerlist and change its equipped weapon
    int playerID = ATOI(1);

    // Change equipped weapon of specified player
    for(auto& player : status.playerList)
    {
        if(player.id == playerID) {
            player.equippedWeapon = ATOI(2);
            break;
        }
    }

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Player Points Changed*/
void wsPlayerPointsChanged(OnlineFunctionParams)
{
    // Args: messageString, server's playerID, new addition
    if(msg.size()<3) return;

    // Go through all playerlist and change its credits
    int playerID = ATOI(1);

    // Change equipped weapon of specified player
    for(auto& player : status.playerList)
    {
        if(player.id == playerID) {
            player.credits += ATOI(2);
            break;
        }
    }

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Entity Moved*/
void wsEntityMoved(OnlineFunctionParams)
{
    // Args: messageString, entityId, Vec3 newPos, [optional] Vec3 lookAt, [optional] Vec3 newRot
    if(msg.size()<5) return;

    int id = ATOI(1);
    Vec3 newPos(ATOF(2), ATOF(3), ATOF(4));

    /*
    Vec3 newLookAt;
    bool changedLookAt = msg.size()>=8;
    if(rotated)
        newLookAt = {(float)ATOF(5), (float)ATOF(6), (float)ATOF(7)};
    */

    Vec3 newRot;
    bool rotated = msg.size()>=11;
    if(rotated)
        newRot = {(float)ATOF(8), (float)ATOF(9), (float)ATOF(10)};

    bool found = false;

    // Search and reassign
    if(!found)
    for(auto& data : status.enemyList)
        if(data.id == id) {
            data.position = newPos;
            if(rotated) data.rotation = newRot;
            found = true;
            break;
        }
    if(!found)
    for(auto& data : status.playerList)
        if(data.id == id) {
            data.position = newPos;
            if(rotated) data.rotation = newRot;
            found = true;
            break;
        }
    if(!found)
    for(auto& data : status.mapElementList)
        if(data.id == id) {
            data.position = newPos;
            if(rotated) data.rotation = newRot;
            found = true;
            break;
        }

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Entity Animation Changed*/
void wsEntityAnimationChanged(OnlineFunctionParams)
{
    // Args: messageString, entityId, string newAnim, bool looped
    if(msg.size()<3) return;

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Entity Health Changed*/
void wsEntityHealthChanged(OnlineFunctionParams)
{
    // Args: messageString, entityId, float change
    if(msg.size()<3) return;

    int id = ATOI(1);
    float change = ATOF(2);

    bool found = false;

    // Search and reassign
    if(!found)
    for(auto& data : status.enemyList)
        if(data.id == id) {
            data.health -= change;
            auto& list = status.enemyList;
            if(data.health <= 0)
                list.erase(
                    std::remove_if(list.begin(), list.end(),
                        [&id](EnemyData& data){return data.id == id;}
                    )
                );
            found = true;
            break;
        }
    if(!found)
    for(auto& data : status.playerList)
        if(data.id == id) {
            data.health -= change;
            if(data.health <= 0)
                data.resetEntity();
            found = true;
            break;
        }

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}


/*Status Applied*/
void wsStatusApplied(OnlineFunctionParams)
{
    // Args: messageString, entityId, int statusApplied, float timeApplied, int applier
    if(msg.size()<5) return;

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Player Jumped*/
void wsPlayerJumped(OnlineFunctionParams)
{
    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Player Dashed*/
void wsPlayerDashed(OnlineFunctionParams)
{
    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Player Shooted*/
void wsPlayerIsShooting(OnlineFunctionParams)
{
    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}
void wsEnemyBulletShooted(OnlineFunctionParams)
{
    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Door Opened*/
void wsDoorOpened(OnlineFunctionParams)
{
    // Args: messageString, doorId, int interactorId, [optional] bool newState (opened/closed)
    if(msg.size()<3) return;

    int doorId = ATOI(1);

    for(auto& data : status.doorList)
        if(data.id == doorId) {
            data.isOpen = true;
            break;
        }

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Pickup Collected*/
void wsPickupCollected(OnlineFunctionParams)
{
    // Args: messageString, idInServer (of the collected pickup), [optional] id of entity that picked this
    if(msg.size()<2) return;

    // Get args
    int pickupId = ATOI(1);

    // Find the pickup in the list
    auto& list = status.pickupList;
    auto it = std::find_if(list.begin(), list.end(),
        [&pickupId](PickupData& data){return data.id == pickupId;}
    );
    if(it == list.end()) return;
    PickupType pickupType = it->pickupType;
    
    // Delete the pickup
    list.erase(it);

    // Append pickup type to the re-sent message
    msg.insert(msg.begin()+2, std::to_string(pickupType));

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Button Interacted*/
void wsButtonInteracted(OnlineFunctionParams)
{
    // Args: messageString, buttonId (or platform)
    //if(msg.size()<2) return;

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Vending Machine Activated*/
void wsVendingMachineActivated(OnlineFunctionParams)
{
    // Args: messageString, buttonId (or platform)
    //if(msg.size()<2) return;

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}


/*Enemies Killed Count Changed*/
void wsEnemiesKilledCountChanged(OnlineFunctionParams)
{
    // Args: messageString, int newCount
    if(msg.size()<2) return;

    // Save count in world status
    status.enemyKillCount = ATOI(1);

    // Resend message to the other clients
    // OPTIONAL
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Round Changed*/
void wsRoundChanged(OnlineFunctionParams)
{
    // Args: messageString, newRound
    //if(msg.size()<2) return;

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Round Finished*/
void wsRoundFinished(OnlineFunctionParams)
{
    // Args: messageString

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}


/*Level Finished*/
void wsLevelFinished(OnlineFunctionParams)
{
    status.mapIndex = -1;

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}


/*Get Level*/
void wsGetLevel(OnlineFunctionParams)
{
    s->send(hdl, "MapLoaded" SEP+std::to_string(status.mapIndex));
}

/*Get World Status*/
void wsGetWorldStatus(OnlineFunctionParams)
{
    // Send doors
    for(auto& data : status.doorList)
        s->send(hdl, 
            "DoorSpawned" SEP+
            std::to_string(data.id)
            +SEP+std::to_string(data.position.x)
            +SEP+std::to_string(data.position.y)
            +SEP+std::to_string(data.position.z)
            +SEP+std::to_string(data.rotation.x)
            +SEP+std::to_string(data.rotation.y)
            +SEP+std::to_string(data.rotation.z)
            +SEP+std::to_string(data.scale.x)
            +SEP+std::to_string(data.scale.y)
            +SEP+std::to_string(data.scale.z)
            +SEP+std::to_string((int)data.isOpen)
            +SEP+std::to_string((int)data.doorType)
            +SEP+std::to_string(data.creditCost)
            +SEP+std::to_string(data.remoteID)
        );

    // Send map elements
    for(auto& data : status.mapElementList)
        s->send(hdl, 
            "MapElementSpawned" SEP+
            std::to_string(data.id)
            +SEP+std::to_string(data.position.x)
            +SEP+std::to_string(data.position.y)
            +SEP+std::to_string(data.position.z)
            +SEP+std::to_string(data.scale.x)
            +SEP+std::to_string(data.scale.y)
            +SEP+std::to_string(data.scale.z)
            +SEP+std::to_string(data.rotation.x)
            +SEP+std::to_string(data.rotation.y)
            +SEP+std::to_string(data.rotation.z)
            +SEP+std::to_string(data.elementType)
            +SEP+std::to_string(data.sharedKeyId)
            +SEP+std::to_string(data.idRemoteToInteractWith)
            +SEP+std::to_string(data.idRemoteToBeInteracted)
            +SEP+std::to_string(data.interactionData)
        );

    // Send other players
    for(auto& data : status.playerList)
        if(data.id >= 0)
            s->send(hdl, 
                "PlayerSpawned" SEP+
                std::to_string(data.id)
                +SEP+std::to_string(data.position.x)
                +SEP+std::to_string(data.position.y)
                +SEP+std::to_string(data.position.z)
                +SEP+data.name
                +SEP+std::to_string(data.health)
                +SEP+std::to_string(data.equippedWeapon)
                +SEP+std::to_string(data.look)
            );

    // Send enemies
    for(auto& data : status.enemyList)
        s->send(hdl, 
            "EnemySpawned" SEP+
            std::to_string(data.id)
            +SEP+std::to_string(data.enemyType)
            +SEP+std::to_string(data.health)
            +SEP+std::to_string(data.isFromRaid)
            +SEP+std::to_string(data.position.x)
            +SEP+std::to_string(data.position.y)
            +SEP+std::to_string(data.position.z)
            +SEP+std::to_string(data.scale.x)
            +SEP+std::to_string(data.scale.y)
            +SEP+std::to_string(data.scale.z)
            +SEP+std::to_string(data.rotation.x)
            +SEP+std::to_string(data.rotation.y)
            +SEP+std::to_string(data.rotation.z)
        );

    // Send pickups
    for(auto& data : status.pickupList)
    {
        std::string result =
            "PickupSpawned" SEP+
            std::to_string(data.id)
            +SEP+std::to_string(data.position.x)
            +SEP+std::to_string(data.position.y)
            +SEP+std::to_string(data.position.z)
            +SEP+std::to_string(data.rotation.x)
            +SEP+std::to_string(data.rotation.y)
            +SEP+std::to_string(data.rotation.z)
            +SEP+std::to_string(data.pickupType)
        ;
        for(auto& val : data.dataValues)
            result += SEP + std::to_string(val);

        s->send(hdl, result);
    }

    // Send round data
    s->send(hdl, "EnemiesKilledCountChanged" SEP+std::to_string(status.enemyKillCount));
    s->send(hdl, "RoundChanged" SEP+std::to_string(status.round.currentRound));

    // Send task data
}

/*Player Connected*/
void wsPlayerConnected(OnlineFunctionParams)
{
   // Args: messageString, string name, int skin, bool isHost
    if(msg.size()<4) 
        return;

    // Base elements
    std::string name { msg[1] };
    int characterSkin { ATOI(2) };
    bool playerJoiningIsHost { (bool)ATOI(3) };

    // If player name is empty, abort connection
    if(msg.size() > 4)
    {
        // Abort connection
        s->send(hdl, "ForceDisconnect" SEP "Player name can't contain ampersand");
        return;
    }
    if(name == "")
    {
        // Abort connection
        s->send(hdl, "ForceDisconnect" SEP "Player name can't be empty");
        return;
    }

    // Check host players to see if this player can join
    // A host cannot join if there is another host already
    // A guest cannot join if there is no host
    bool teamHasHost { false };
    auto& playerList { status.playerList };
    if(!playerList.empty())
    {
        for(auto& player : playerList)
        {
            if(player.isHost)
            {
                teamHasHost = true;
                break;
            }
        }
    }
    // Condition is:
    // if (teamHasHost && playerJoiningIsHost) OR (!teamHasHost && !playerJoiningIsHost)
    // So condition is true if both bools are the same
    // teamHasHost == playerJoiningIsHost
    if( teamHasHost == playerJoiningIsHost )
    {
        // Abort connection
        std::string reason = teamHasHost ? "Can't have more than one host" : "There is no host in the server";
        s->send(hdl, "ForceDisconnect" SEP + reason);
        return;
    }

    PlayerData data;
    data.name = name;
    data.isHost = playerJoiningIsHost;
    data.look = characterSkin;
    data.hdl = hdl;

    // Add new data to list
    playerList.push_back(data);

    // Tell the other clients to spawn this element
    std::string result = "PlayerConnected";
    for(unsigned int i=1; i<msg.size(); i++)
        result += SEP+msg[i];
    
    s->broadcast_all_others(hdl, result);
}

/*Player Disconnected*/
void wsPlayerDisconnected(OnlineFunctionParams)
{
    //auto& playerNames = status.playerNamesList;
    //auto& name = msg[1];
    //playerNames.erase(
    //    std::remove(playerNames.begin(), playerNames.end(), name)
    //);

    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Selection Changed*/
void wsSelectionChanged(OnlineFunctionParams)
{
    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Level Selected*/
void wsLevelSelected(OnlineFunctionParams)
{
    // Resend message to the other clients
    auto message = reassembleMessage(msg);
    s->broadcast_all_others(hdl, message);
}

/*Get Player List*/
void wsGetPlayerList(OnlineFunctionParams)
{
    // Send player names
    auto& list = status.playerList;
    for(auto& data : list)
        s->send(hdl, "PlayerConnected" SEP + data.name + SEP + std::to_string(data.look));

    if(status.mapIndex >= 0)
    {
        s->send(hdl, "SelectionChanged" SEP + std::to_string(status.mapIndex));
        s->send(hdl, "LevelSelected");
    }
}

/*Parse Message*/
// Parses the message using a delimiter
StrVec parseMessage(std::string msg)
{
    static const std::string delimiter = SEP;
    StrVec result{};

    size_t pos = 0;
    std::string token;
    while ((pos = msg.find(delimiter)) != std::string::npos) {
        token = msg.substr(0, pos);
        result.push_back(token);
        msg.erase(0, pos + delimiter.length());
    }
    result.push_back(msg);

    return result;
}

std::string reassembleMessage(const StrVec& v) {
    std::string result="";

    if( v.empty() )
        return result;

    result += v[0];

    for(uint32_t i = 1;i < v.size();++i)
        result += SEP + v[i];

    return result;
}


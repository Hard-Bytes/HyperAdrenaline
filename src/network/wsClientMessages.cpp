#include "wsClientMessages.hpp"
#include "../context/gameContext.hpp"
#include "../sharedTypes/serverEnums.hpp"
#include "../system/interactionBehaviours.hpp"
#include "../components/healthCmpFunc.hpp"
#include "../components/weaponCmpFunc.hpp"
#include "../components/inventoryCmpFunc.hpp"

/*Vinculate Entity*/
void wsVinculateEntity(OnlineClientFunctionParams)
{
    // Minimum args check
    if(msg.size()<3) return;

    int localId = ATOI(1);
    int serverId = ATOI(2);

    // Search for entity in game and assign server id
    TypeComponent* type = gctx->entityManager.getComponentByID<TypeComponent>(localId);
    if(type)
        type->idInServer = serverId;
}


/*Map Loaded*/
// The host loaded a map, so the others must load this map
// Also cleans the current game state
void wsMapLoaded(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, level index (int)
    if(msg.size()<2) return;

    gctx->levelManager.flagReloadLevel = true;
    gctx->levelManager.forceCurrentLevel(ATOI(1));
}


/*Player Spawned*/
void wsPlayerSpawned(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, idInServer, Vec3 pos, string name, float currentHealth, weapon(int), enum look
    if(msg.size()<9 || !gctx->webMessageInterface.getAllowEntitySpawning())
        return;

    // Get parameters
    int idInServer = ATOI(1);
    Vector3f pos{ (float)ATOF(2), (float)ATOF(3), (float)ATOF(4) };
    //std::string name = msg[5];
    float currentHealth = ATOF(6);
    int weaponId = ATOI(7);
    PlayerLook look = (PlayerLook)ATOI(8);

    // Check if this entity already exists
    int repeatedId = getLocalIdFromServerId(idInServer, gctx);
    if(repeatedId >= 0) return;

    // Create entity
    int localId = gctx->gameManager.createNPCPlayerEntity(pos, look);

    // Assign server id
    TypeComponent* type = gctx->entityManager.getComponentByID<TypeComponent>(localId);
    if(type) type->idInServer = idInServer;

    // Modify health
    HealthComponent* health = gctx->entityManager.getComponentByID<HealthComponent>(localId);
    health->health = currentHealth;

    // Equip weapon
    if(weaponId >= 0)
    {
        WeaponComponent* weaponCmp = gctx->entityManager.getComponentByID<WeaponComponent>(localId);
        cf::equipWeapon(*weaponCmp, *gctx, (WeaponID)weaponId, true, false);
    }
}

/*Enemy Spawned*/
void wsEnemySpawned(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, idInServer, type(int), float health, bool isFromRaid Vec3 pos, [optional] Vec3 scale, Vec3 rot
    if(msg.size()<8 || !gctx->webMessageInterface.getAllowEntitySpawning())
        return;

    // Get parameters
    int idInServer = ATOI(1);
    int enemyType = ATOI(2);
    float currentHealth = ATOF(3);
    bool isfromRaid = ATOI(4);
    Vector3f pos{ (float)ATOF(5), (float)ATOF(6), (float)ATOF(7) };

    // Get optional parameters
    Vector3f scale{1.f,1.f,1.f};
    Vector3f rot{0.f,0.f,0.f};
    if(msg.size() >= 11)
        scale = Vector3f{ (float)ATOF(8), (float)ATOF(9), (float)ATOF(10) };
    if(msg.size() >= 14)
        rot = Vector3f{ (float)ATOF(11), (float)ATOF(12), (float)ATOF(13) };

    // Create entity
    int localId = gctx->gameManager.createEnemyEntity(enemyType, pos, scale, rot, 0,0,isfromRaid);

    // Modify health
    HealthComponent* health = gctx->entityManager.getComponentByID<HealthComponent>(localId);
    health->health = currentHealth;

    // Assign server id
    TypeComponent* type = gctx->entityManager.getComponentByID<TypeComponent>(localId);
    if(type) type->idInServer = idInServer;
}

/*Door Spawned*/
void wsDoorSpawned(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, idInServer, Vec3 pos, Vec3 rot, Vec3 scale, bool isOpen, type(int) [optional]price/key (int), [optional] remoteId (int)
    if(msg.size()<13 || !gctx->webMessageInterface.getAllowEntitySpawning())
        return;

    // Get parameters
    int idInServer = ATOI(1);
    Vector3f pos{ (float)ATOF(2), (float)ATOF(3), (float)ATOF(4) };
    Vector3f rot{ (float)ATOF(5), (float)ATOF(6), (float)ATOF(7) };
    Vector3f scale{ (float)ATOF(8), (float)ATOF(9), (float)ATOF(10) };
    bool isOpen = ATOI(11);
    int doorType = ATOI(12);

    int localId = -1;

    int price = 0;
    int remoteId = -1;
    if(msg.size()>13)
        price = ATOI(13);
    if(msg.size()>14)
        remoteId = ATOI(14);

    // Create entity
    switch((DoorType)doorType)
    {
        case D_InteractableDoor:
            localId = gctx->gameManager.createInteractuableDoorEntity(pos, scale, rot, false, remoteId);
            break;
        case D_CreditsDoor:
            localId = gctx->gameManager.createInteractuableDoorWithPointsEntity(pos, scale, rot, price);
            break;
        case D_KeyDoor:
            localId = gctx->gameManager.createInteractuableDoorWithCardEntity(pos, scale, rot, price, remoteId);
            break;
        case D_RemoteDoor:
            localId = gctx->gameManager.createInteractuableDoorEntity(pos, scale, rot, true, remoteId);
            break;
        case D_KillsDoor:
            localId = gctx->gameManager.createInteractuableEnemyKillDoorEntity(pos, scale, rot, price);
            break;
    }

    // Assign server id
    TypeComponent* type = gctx->entityManager.getComponentByID<TypeComponent>(localId);
    if(type) type->idInServer = idInServer;

    // Open if necessary
    if(isOpen && localId >= 0)
    {
        InteractableComponent* interactable = gctx->entityManager.getComponentByID<InteractableComponent>(localId);
        if(interactable) interactionOpenDoor(*interactable, UNREACHABLE_ENTITY, *gctx);
    }
}

/*Pickup Spawned*/
void wsPickupSpawned(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, idInServer, Vec3 pos, Vec3 rot, type(int) [optional] Infinite floats
    if(msg.size()<9 || !gctx->webMessageInterface.getAllowEntitySpawning())
        return;

    // Get parameters
    int idInServer = ATOI(1);
    Vector3f pos{ (float)ATOF(2), (float)ATOF(3), (float)ATOF(4) };
    Vector3f size{ (float)ATOF(5), (float)ATOF(6), (float)ATOF(7) };
    int pickupType = ATOI(8);

    int localId = -1;

    int kitParentIdInServer = -1;
    bool parentIsVendingMachine = false;
    if(msg.size() >= 12)
    {
        kitParentIdInServer = ATOF(10);
        parentIsVendingMachine = ATOI(11);
    }
    int kitParentIdLocal = getLocalIdFromServerId(kitParentIdInServer, gctx);

    // Create entity
    switch((PickupType)pickupType)
    {
        case PK_Key:
            localId = gctx->gameManager.createInteractuableCardEntity(pos, size, ATOI(9));
            break;
        case PK_Weapon:
            localId = gctx->gameManager.createInteractuableWeaponEntity(pos, size, ATOI(9));
            break;
        case PK_Medkit:
            localId = gctx->gameManager.createKitEntity(pos, size, 1, kitParentIdLocal, parentIsVendingMachine);
            break;
        case PK_Ammo:
            localId = gctx->gameManager.createKitEntity(pos, size, 2, kitParentIdLocal, parentIsVendingMachine);
            break;
    }

    // Assign server id
    TypeComponent* type = gctx->entityManager.getComponentByID<TypeComponent>(localId);
    if(type) type->idInServer = idInServer;
}

/*Map Element Spawned*/
void wsMapElementSpawned(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, idInServer, Vec3 pos, Vec3 scale, Vec3 rot, type(int) [optional] 4 ints (sharedKeyId, remoteIdWith, remoteidBe, value)
    if(msg.size()<12 || !gctx->webMessageInterface.getAllowEntitySpawning())
        return;

    // Get parameters
    int idInServer = ATOI(1);
    Vector3f pos{ (float)ATOF(2), (float)ATOF(3), (float)ATOF(4) };
    Vector3f scale{ (float)ATOF(5), (float)ATOF(6), (float)ATOF(7) };
    Vector3f rot{ (float)ATOF(8), (float)ATOF(9), (float)ATOF(10) };
    int mapElementType = ATOI(11);

    // Get optional parameters
    int keyId = -1;
    int remoteIdToInteractWith = -1;
    int remoteIdToBeInteracted = -1;
    float interactionData = -1;
    if(msg.size()>=13)
        keyId = ATOI(12);
    if(msg.size()>=14)
        remoteIdToInteractWith = ATOI(13);
    if(msg.size()>=15)
        remoteIdToBeInteracted = ATOI(14);
    if(msg.size()>=16)
        interactionData = ATOF(15);

    int localId = -1;

    // Create entity
    switch((MapElementType)mapElementType)
    {
        case ME_Button:
            localId = gctx->gameManager.createInteractuableRemoteEntity(pos, scale, remoteIdToInteractWith);
            break;
        case ME_ButtonKeyControlled:
            localId = gctx->gameManager.createInteractuableRemoteWithCardEntity(pos, scale, keyId, remoteIdToInteractWith, rot);
            break;
        case ME_MovingPlatform:
            localId = gctx->gameManager.createInteractuablePlatform(pos, scale, false, remoteIdToBeInteracted);
            break;
        case ME_MovingPlatformRemote:
            localId = gctx->gameManager.createInteractuablePlatform(pos, scale, true, remoteIdToBeInteracted);
            break;
        case ME_MedkitBase:
            localId = gctx->gameManager.createPadEntity(pos, scale, 1);
            break;
        case ME_AmmoBase:
            localId = gctx->gameManager.createPadEntity(pos, scale, 2);
            break;
        case ME_MedkitVendingMachine:
            localId = gctx->gameManager.createVendingMachine(pos, scale, rot, 1, interactionData);
            break;
        case ME_AmmoVendingMachine:
            localId = gctx->gameManager.createVendingMachine(pos, scale, rot, 2, interactionData);
            break;
        case ME_Solid:
            localId = gctx->gameManager.createSolidEntity(pos, scale);
            break;
        case ME_KillingArea:
            localId = gctx->gameManager.createKillingZoneEntity(pos, scale);
            break;
        case ME_DamagingArea:
            localId = gctx->gameManager.createDamagingZoneEntity(pos, scale, interactionData);
            break;
        case ME_LevelExit:
            localId = gctx->gameManager.createExitEntity(pos, scale);
            break;
    }

    // Assign server id
    TypeComponent* type = gctx->entityManager.getComponentByID<TypeComponent>(localId);
    if(type) type->idInServer = idInServer;
}


/*Player Weapon Changed*/
void wsPlayerWeaponChanged(OnlineClientFunctionParams)
{
    int idInServer = ATOI(1);
    int weapon = ATOI(2);

    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        WeaponComponent* weaponCmp = gctx->entityManager.getComponentByID<WeaponComponent>(id);
        
        if(weaponCmp) 
            cf::equipWeapon(*weaponCmp, *gctx, (WeaponID)weapon, true, false);
    }
}

/*Player Points Changed*/
void wsPlayerPointsChanged(OnlineClientFunctionParams)
{
    int idInServer = ATOI(1);
    int newPoints = ATOI(2);

    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        auto inv = gctx->entityManager.getComponentByID<InventoryComponent>(id);
        if(inv) cf::addCredits(*inv, newPoints, *gctx, false);
    }
}

/*Entity Moved*/
void wsEntityMoved(OnlineClientFunctionParams)
{
    // Args: messageString, idInServer, Vec3 pos, [optional] Vec3 lookAt, [optional] Vec3 rot
    if(msg.size()<5) return;

    int idInServer = ATOI(1);
    Vector3f newPos{ (float)ATOF(2), (float)ATOF(3), (float)ATOF(4) };

    bool changedTarget = msg.size()>=8;
    Vector3f newLookAt;
    if(changedTarget)
        newLookAt = { (float)ATOF(5), (float)ATOF(6), (float)ATOF(7) };

    bool rotated = msg.size()>=11;
    Vector3f newRot;
    if(rotated)
        newRot = { (float)ATOF(8), (float)ATOF(9), (float)ATOF(10) };

    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        // Get local offset
        CollisionComponent* coll = gctx->entityManager.getComponentByID<CollisionComponent>(id);
        Vector3f offset;
        if(coll) offset = coll->offset;

        // Assign position
        NodeComponent* node = gctx->entityManager.getComponentByID<NodeComponent>(id);
        if(node)
        {
            node->node->setPosition(newPos+offset);
            if(rotated) node->node->setRotation(Vector3f(0,newRot.y,0));
            if(changedTarget) 
            {
                AIComponent* aiCmp = gctx->entityManager.getComponentByID<AIComponent>(id);
                if(aiCmp) aiCmp->lookAt = newLookAt;

                auto* typeCmp = gctx->entityManager.getComponentByID<TypeComponent>(id);
                if(!(typeCmp && typeCmp->type == ENT_PLAYER)) return;
                newLookAt -= newPos;
                newLookAt.y = 0;
                newLookAt = newLookAt.getHorizontalAngle();
                newLookAt.y += 180;
                node->node->setRotation(newLookAt);
            }
        } 
    }
}

/*Entity Animation Changed*/
void wsEntityAnimationChanged(OnlineClientFunctionParams)
{
    int idInServer = ATOI(1);
    bool looped = ATOI(3);
    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        auto* nodeCmp { gctx->entityManager.getComponentByID<NodeComponent>(id) };
        nodeCmp->node->playAnimation(msg[2], looped);
    }
}

/*Entity Health Changed*/
void wsEntityHealthChanged(OnlineClientFunctionParams)
{
    int idInServer = ATOI(1);
    float change = ATOF(2);

    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        HealthComponent* health = gctx->entityManager.getComponentByID<HealthComponent>(id);
        if(health) 
            cf::doDamage(*health, *gctx, change, 0.0f, false);
    }
}


/*Status Applied*/
void wsStatusApplied(OnlineClientFunctionParams)
{
    // Args: messageString, entityId, int statusApplied, float timeApplied, int applier
    if(msg.size()<5) return;

    int idInServer = ATOI(1);
    StatusType statusType = (StatusType)ATOI(2);
    float timeApplied = ATOF(3);
    int applierServerId = ATOI(4); // Id of the entity that applied the effect

    int id = getLocalIdFromServerId(idInServer, gctx);
    int applierLocalId = getLocalIdFromServerId(applierServerId, gctx);
    StatusComponent* statusComponent = gctx->entityManager.getComponentByID<StatusComponent>(id);
    if(statusComponent)
    {
        Stat statStruct = {statusType, applierLocalId, timeApplied};
        statusComponent->status.push_back(statStruct);
    }
}


/*Player Jumped*/
void wsPlayerJumped(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, playerId (in server)
    if(msg.size()<2) return;

    // TODO
    // We should get position but sound is not 3D

    // Play sound
    //gctx->audioManager.play3DSound("");
}

/*Player Dashed*/
void wsPlayerDashed(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, playerId (in server)
    if(msg.size()<2) return;

    // Get position and speed to throw sound
    int playerLocalId = getLocalIdFromServerId(ATOI(1), gctx);
    if(playerLocalId < 0) return;
    auto nodeCmp = gctx->entityManager.getComponentByID<NodeComponent>(playerLocalId);
    Vector3f position { 0, 0, 0 };
    if(nodeCmp) position = nodeCmp->node->getPosition();

    // Play sound
    gctx->audioManager.play3DSound(
        SOUND_SFX_DASH
        , SOUNDS_SFX_COMBAT
        , position
    );
}

/*Player Is Shooting*/
void wsPlayerIsShooting(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, playerId (in server), bool isShooting
    if(msg.size()<3) return;

    // Set weapon "isFiring" property to true
    int idInServer = ATOI(1);
    bool isShooting = ATOI(2);
    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        // Activate NPC's "toy" weapon (damage is zero, but makes lights and sound)
        WeaponComponent* weaponCmp = gctx->entityManager.getComponentByID<WeaponComponent>(id);
        if(weaponCmp)
            weaponCmp->weapons[weaponCmp->index].isFiring = isShooting;
    }
}

/*Enemy Shooted*/
void wsEnemyBulletShooted(OnlineClientFunctionParams)
{
    // Minimum args check
    // Args: messageString, vec3 pos, vec3 size, vec3 rot, vec3 speed, float damage, float timeAlive, string sound
    if(msg.size()<16) return;

    // Get parameters
    Vector3f pos{ (float)ATOF(1), (float)ATOF(2), (float)ATOF(3) };
    Vector3f size{ (float)ATOF(4), (float)ATOF(5), (float)ATOF(6) };
    Vector3f rot{ (float)ATOF(7), (float)ATOF(8), (float)ATOF(9) };
    Vector3f speed{ (float)ATOF(10), (float)ATOF(11), (float)ATOF(12) };
    float damage = ATOF(13);
    float timeAlive = ATOF(14);
    std::string sound = msg[15];

    gctx->gameManager.createEnemyBullet(pos,size,rot,speed,damage,timeAlive,sound);
}


/*Door Opened*/
void wsDoorOpened(OnlineClientFunctionParams)
{
    int idInServer = ATOI(1);
    //int interactorIdInServer = ATOI(2);
    //int localInteractorId = getLocalIdFromServerId(interactorIdInServer, gctx);
    //bool newState = ATOI(3);

    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        InteractableComponent* interactable = gctx->entityManager.getComponentByID<InteractableComponent>(id);
        if(interactable) interactionOpenDoor(*interactable, UNREACHABLE_ENTITY, *gctx);
    }
}

/*Pickup Collected*/
void wsPickupCollected(OnlineClientFunctionParams)
{
    // Get pickup's id
    int idInServer = ATOI(1);
    PickupType pickupType = (PickupType)ATOI(2);
    int id = getLocalIdFromServerId(idInServer, gctx);

    int interactorId = -1;
    if(msg.size() >= 4)
    {
        int interactorServerId = ATOI(3);
        interactorId = getLocalIdFromServerId(interactorServerId, gctx);
    }

    // If we're host and it's a healkit or an ammo kit, tell their pad to regenerate
    if(pickupType == PK_Medkit || pickupType == PK_Ammo)
    {
        auto* pkInter = gctx->entityManager.getComponentByID<InteractableComponent>(id);
        if(pkInter)
        {
            auto* padAI = gctx->entityManager.getComponentByID<AIComponent>(pkInter->idRemoteToInteractWith);
            if(padAI) padAI->hasKit = 2;
        }
    }
    else if(pickupType == PK_Key && interactorId >= 0)
    {
        // Recover key's interactable (which contains key id) and player's inventory (to add the key id)
        auto keyInteractable = gctx->entityManager.getComponentByID<InteractableComponent>(id);
        auto inventory = gctx->entityManager.getComponentByID<InventoryComponent>(interactorId);
        if(inventory && keyInteractable)
            cf::addKey(*inventory, keyInteractable->sharedKeyId, *gctx);
    }

    // Destroy pickup
    gctx->entityManager.markAsDead(id);
}

/*Button Interacted*/
void wsButtonInteracted(OnlineClientFunctionParams)
{
    int idInServer = ATOI(1);
    //int interactorIdInServer = ATOI(2);
    //int localInteractorId = getLocalIdFromServerId(interactorIdInServer, gctx);

    int id = getLocalIdFromServerId(idInServer, gctx);
    if(id >= 0)
    {
        InteractableComponent* interactable = gctx->entityManager.getComponentByID<InteractableComponent>(id);
        if(interactable) interactable->interaction(*interactable, UNREACHABLE_ENTITY, *gctx);
    }
}

/*Vending Machine Activated*/
void wsVendingMachineActivated(OnlineClientFunctionParams)
{
    // Args: messageString, int idInServer, int interactorID
    if(msg.size()<3) return;

    int idInServer = ATOI(1);
    int id = getLocalIdFromServerId(idInServer, gctx);
    int interactorID = ATOI(2);
    int localInteractorID = getLocalIdFromServerId(interactorID, gctx);

    if(id >= 0)
    {
        // Disable kit spending
        auto* machineInteractionCmp = gctx->entityManager.getComponentByID<InteractableComponent>(id);
        if(machineInteractionCmp)
            interactionRemoteEntityInteract(*machineInteractionCmp, localInteractorID, *gctx);

        // Reset time
        int padID = machineInteractionCmp->idRemoteToInteractWith;
        auto* aiCmp = gctx->entityManager.getComponentByID<AIComponent>(padID);
        if(aiCmp) aiCmp->timeWithoutKit = 0;
    }
}


/*Enemies Killed Count Changed*/
void wsEnemiesKilledCountChanged(OnlineClientFunctionParams)
{
    // Args: messageString, int newCount
    if(msg.size()<2) return;

    // Save count in world status
    gctx->gameManager.killedMapEnemies = ATOI(1);
	gctx->hudManager.forceInteractableInfoRecalculation();
}

/*Round Changed*/
void wsRoundChanged(OnlineClientFunctionParams)
{
    //int newRound = ATOI(1);

    // Assign new round to somewhere using Game Context
}

/*Round Finished*/
void wsRoundFinished(OnlineClientFunctionParams)
{

}


/*Level Finished*/
void wsLevelFinished(OnlineClientFunctionParams)
{
    // Mark the level as finished for everyone
    if(gctx->networkClient.isHost())
        gctx->levelManager.flagLevelFinished = true;
}

/*Force Disconnection*/
// Server has told us to disconnect, obey
void wsForceDisconnect(OnlineClientFunctionParams)
{
    // Args: messageString, disconnection reason
    auto disconnectionReason = msg[1];
    gctx->webMessageInterface.setDisconnectionReason(disconnectionReason);
    gctx->networkClient.close("Kicked by server ("+disconnectionReason+")");
}


/*Player Connected*/
void wsPlayerConnected(OnlineClientFunctionParams)
{
    auto& playerNames = gctx->gameManager.getPlayerNames();
    auto& name = msg[1];
    auto it = std::find(playerNames.begin(), playerNames.end(), name);
    if(it == playerNames.end()) gctx->gameManager.addPlayerNameToList(name);
}

/*Player Disconnected*/
void wsPlayerDisconnected(OnlineClientFunctionParams)
{
    // Args: messageString, string playerName, int inGameId
    std::string playerName = msg[1];
    int serverPlayerId = ATOI(2);
    int localPlayerId = getLocalIdFromServerId(serverPlayerId, gctx);

    if(playerName == "") return;

    // Remove from online list
    gctx->gameManager.removePlayerNameFromList(playerName);

    // If player is not valid it does not exist anymore, return
    if(localPlayerId < 0) return;

    // Recover keys from player's inventory
    std::vector<int> keys;
    auto invCmp = gctx->entityManager.getComponentByID<InventoryComponent>(localPlayerId);
    if(invCmp)
    {
        auto& keysTmp = invCmp->keys;
        keys.reserve(keysTmp.size());
        for(auto key : keysTmp) keys.push_back(key);
    }

    // Remove player's body if in-game
    gctx->entityManager.markAsDead(localPlayerId);

    // Assign keys to host
    if(gctx->networkClient.isHost() && keys.size() > 0)
    {
        int hostId = gctx->gameManager.getLocalPlayerID();
        auto hostInventory = gctx->entityManager.getComponentByID<InventoryComponent>(hostId);
        if(hostInventory) 
            for(auto key : keys)
                cf::addKey(*hostInventory, key, *gctx);
    }
}

/*Selection Changed*/
void wsSelectionChanged(OnlineClientFunctionParams)
{
    gctx->gameManager.m_menuSelection = ATOI(1);
}

/*Level Selected*/
void wsLevelSelected(OnlineClientFunctionParams)
{
    gctx->gameManager.m_selectionAccepted = true;
}

int getLocalIdFromServerId(int idInServer, GameContext* gctx)
{
    if(idInServer < 0) return INVALID_ENTITY;

    auto types = gctx->entityManager.getComponentVector<TypeComponent>();
    auto it = std::find_if(types.begin(), types.end(), 
        [&idInServer](TypeComponent& type){return type.idInServer == idInServer;}
    );
    if(it != types.end())
        return it->getEntityID();
    return INVALID_ENTITY;
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

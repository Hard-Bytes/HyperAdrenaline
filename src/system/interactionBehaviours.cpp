#include "interactionBehaviours.hpp"
#include "respawn.hpp"
#include "../components/entity.hpp"
#include "../components/healthCmpFunc.hpp"
#include "../components/weaponCmpFunc.hpp"
#include "../components/inventoryCmpFunc.hpp"
#include "../util/raidTimes.hpp"

/*Interaction: Death*/
// Interactor touched a death area, so it dies
void interactionInKillingArea(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(interactorID);
    if(health) cf::kill(*health, gctx); // Oh no, interactor is kill
}

/*Interaction: Damage*/
// Interactor touched a damaging area, so it takes damage
void interactionInDamagingArea(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(interactorID);
    TypeComponent* typeInteractor = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
    TypeComponent* typeArea = gctx.entityManager.getComponentByID<TypeComponent>(interacted.getEntityID());
    if(health) 
    {  
        if(typeArea->type == ENT_EFFECTAREA || (typeInteractor->type == ENT_ENEMY && (typeArea->type == ENT_ATTACKAREA || typeArea->type == ENT_EXPLOSIONAREA)) || (typeInteractor->type == ENT_PLAYER && typeArea->type == ENT_ENEMYATTACKAREA) )
        {   
            float oldHealth=health->health;
            cf::doDamage(*health, gctx, interacted.interactionData, 1.0f);
            std::vector<int> playerIDs = gctx.gameManager.getPlayerIDs();
            if( typeInteractor->type == ENT_ENEMY  && oldHealth>0 && health->health <=0 && playerIDs.size() > 0)
            {
                int playerID = playerIDs[0];
                auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>(playerID);

                InteractableComponent* enemyInt = gctx.entityManager.getComponentByID<InteractableComponent>(interactorID);
                cf::addCredits(*playerInv, enemyInt->sharedKeyId, gctx);
            }
            if(typeInteractor->type == ENT_PLAYER) gctx.audioManager.playSound(SOUND_SFX_ENEMYATTACK, SOUNDS_SFX_COMBAT);
        }
        if( typeArea->type == ENT_EXPLOSIONAREA && typeInteractor->type == ENT_PLAYER )
        {
            cf::doDamage(*health, gctx, interacted.interactionData/10, 0.5f);
        }
    }
}

/*Interaction: Damage*/
// Interactor touched a damaging area, so it takes damage
void interactionInExplosionArea(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(interactorID);
    TypeComponent* typeInteractor = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
    if(health) 
    {  
 
            float oldHealth=health->health;
            cf::doDamage(*health, gctx, interacted.interactionData, 1.0f);
            std::vector<int> playerIDs = gctx.gameManager.getPlayerIDs();
            if( typeInteractor->type == ENT_ENEMY  && oldHealth>0 && health->health <=0 && playerIDs.size() > 0)
            {
                int playerID = playerIDs[0];
                auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>(playerID);

                InteractableComponent* enemyInt = gctx.entityManager.getComponentByID<InteractableComponent>(interactorID);
                cf::addCredits(*playerInv, enemyInt->sharedKeyId, gctx);
            }
            if(typeInteractor->type == ENT_PLAYER) gctx.audioManager.playSound(SOUND_SFX_ENEMYATTACK, SOUNDS_SFX_COMBAT); 
    }
}


/*Interaction: Damage*/
// Interactor touched a damaging area, so it makes an explosion and suicide itself
void interactionExplosionArea(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    
    TypeComponent* typeArea = gctx.entityManager.getComponentByID<TypeComponent>(interacted.getEntityID());
    int interactorType=ENT_OTHER;
    if(interactorID == UNREACHABLE_ENTITY){
        interactorType=ENT_ENEMY;
    }else{
        TypeComponent* typeInteractor = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
        interactorType=typeInteractor->type;
    }

    NodeComponent* entity = gctx.entityManager.getComponentByID<NodeComponent>(interacted.getEntityID());
    InteractableComponent* interact = gctx.entityManager.getComponentByID<InteractableComponent>(interacted.getEntityID());
    if(typeArea->type == ENT_EFFECTAREA ||  (interactorType == ENT_ENEMY && typeArea->type == ENT_ATTACKAREA) || (interactorType == ENT_PLAYER && typeArea->type == ENT_ENEMYATTACKAREA) )
    {   
        gctx.entityManager.markAsDead(interacted.getEntityID());
        if(typeArea->type == ENT_ATTACKAREA)
        {            
            BehaviourComponent* behCmp = gctx.entityManager.getComponentByID<BehaviourComponent>(interacted.getEntityID());
            gctx.gameManager.createExpandableDamagingZonePerTimeEntity(
                entity->node->getPosition(),
                {2.0,2.0,2.0},
                {entity->node->getRotation().x,entity->node->getRotation().y,0},
                {0,0,0},
                interact->interactionData,
                0.4,
                "",
                behCmp->soundExplosion
            );
            //Explosion Sound
            if(behCmp->sound.size()>0)
            {
                gctx.audioManager.setSoundParameter(behCmp->sound,"isFliying",0, SOUNDS_SFX_COMBAT); 
            }
        }

        if(typeArea->type == ENT_ENEMYATTACKAREA)
        {
            HealthComponent *health = gctx.entityManager.getComponentByID<HealthComponent>(interactorID);
            //Make Explosion Area when enemies Attack
            cf::doDamage(*health, gctx, interacted.interactionData,interact->interactionData);

            //Sound attack
            //Pending to discuss: Where the sound sounds
            gctx.audioManager.play3DSound(SOUND_SFX_ENEMYHIT, SOUNDS_SFX_COMBAT, entity->node->getPosition());
        }
    }
    
}


/*Interaction: Damage*/
// Interactor touched a damaging area, so it makes an explosion and suicides itself
void interactionKamikazeSelfDestruct(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    NodeComponent* entity = gctx.entityManager.getComponentByID<NodeComponent>(interacted.getEntityID());
    InteractableComponent* interact = gctx.entityManager.getComponentByID<InteractableComponent>(interacted.getEntityID());
    //BehaviourComponent* behCmp = gctx.entityManager.getComponentByID<BehaviourComponent>(interacted.getEntityID());

    gctx.gameManager.createExpandableDamagingZonePerTimeEntity(
        entity->node->getPosition(),
        {2.f,2.f,2.f},
        {entity->node->getRotation().x,entity->node->getRotation().y,0},
        {0,0,0},
        interact->interactionData,
        0.5,
        "",
        "",
        ENT_ENEMYATTACKAREA
    );
    //if(behCmp->sound.size()>0)
        //gctx.audioManager.setSoundParameter(behCmp->sound,"isFliying",0);    
    
}

/*Interaction: Level Finished*/
// Someone activated the "level finished event" so the level ends
void interactionLevelFinished(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    gctx.levelManager.flagLevelFinished = true;
}

/*Interaction: Open Door*/
// Makes the door inv and eliminate its hitbox
void interactionOpenDoor(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    gctx.gameManager.spawnArray.insert(gctx.gameManager.spawnArray.end(), interacted.spawnPointsInteract.begin(),  interacted.spawnPointsInteract.end());   
    gctx.gameManager.timeWithoutMaxEnemies = gctx.gameManager.maxtimeWithoutMaxEnemies;
    if(interacted.isRaidZone==true)
        gctx.gameManager.enemiesInARaid+=2;
    
    gctx.gameManager.isRaidZone=interacted.isRaidZone;
    gctx.gameManager.timeWithoutRaid += OPEN_DOOR;
    const NodeComponent* node= gctx.entityManager.getComponentByID<NodeComponent>(interacted.getEntityID());

    node->node->playAnimation("open", false);
    gctx.graphicsEngine.disableAllCollisionsOf(node->node);
    auto* healthCmp { gctx.entityManager.getComponentByID<HealthComponent>(interacted.getEntityID()) };
    cf::kill(*healthCmp, gctx, false);
    

    // Adjust ambience level
    gctx.gameManager.dangerLevel++;
    gctx.audioManager.setSoundParameterForType(SOUND_PARAM_DANGERLEVEL, gctx.gameManager.dangerLevel, SOUNDS_MUSIC);

    // Send online event only if entity is valid
    sendOnlineMessage("DoorOpened", interacted, interactorID, gctx);
}

/*Interaction: Platform Control*/
// activate the platform movement
void interactionPlatformControl(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    if(gctx.networkClient.isHost())
    {
        CollisionComponent* collisionComponent = gctx.entityManager.getComponentByID<CollisionComponent>(interacted.getEntityID());
        AIComponent* aiComponent = gctx.entityManager.getComponentByID<AIComponent>(interacted.getEntityID());

        Vector3f direccion= aiComponent->posF - aiComponent->posI;
        direccion /= collisionComponent->baseSpeed;
        direccion.normalize();
        collisionComponent->speed = direccion * collisionComponent->baseSpeed;
    }

    // Send online event only if entity is valid
    sendOnlineMessage("ButtonInteracted", interacted, interactorID, gctx);
}

/*Interaction: Interaction Remote Interact*/
// activate the interaction of other interactable using local id
void interactionRemoteEntityInteract(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    int idtoInteract = interacted.idRemoteToInteractWith;

    InteractableComponent* otherInteractable = gctx.entityManager.getComponentByID<InteractableComponent>(idtoInteract);
    if(otherInteractable)
        otherInteractable->interaction(*otherInteractable, interacted.getEntityID(), gctx);
}

/*Interaction: Interaction Remote Interact*/
// activate the interaction of other interactable using remote id
void interactionRemoteInteract(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    // Activate all remotes that share our remote id
    auto& allInteractables = gctx.entityManager.getComponentVector<InteractableComponent>();
    for(auto otherInteractable : allInteractables)
        if(otherInteractable.idRemoteToBeInteracted == interacted.idRemoteToInteractWith)
            otherInteractable.interaction(otherInteractable, interacted.getEntityID(), gctx);
}

/*Interaction: Interaction Remote Interact With Card*/
// activate the interaction of other interactable
void interactionRemoteWithCardInteract(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    // Activate all remotes that share our remote id
    auto& allInteractables = gctx.entityManager.getComponentVector<InteractableComponent>();
    for(auto otherInteractable : allInteractables)
        if(otherInteractable.idRemoteToBeInteracted == interacted.idRemoteToInteractWith)
            otherInteractable.interaction(otherInteractable, interactorID, gctx);
}


/*Interaction: Heal Kit*/
// Interactor Heals with touch
void interactionHealKit(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    // Only player interaction control
    TypeComponent* tipo = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
    if(tipo->type!=ENT_PLAYER)return;

    HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(interactorID);
    if(!health) return;

    //Sound Interaction
    gctx.audioManager.playSound(SOUND_SFX_PICKUP_HEAL, SOUNDS_SFX_AMBIENCE);
    if(health->health<=health->maxHealth/2)
        gctx.audioManager.playSound(SOUND_VFX_LIAM_PICKUP_HEAL, SOUNDS_VOICE);

    cf::doDamage( *health, gctx, -interacted.interactionData );
    gctx.gameManager.timeWithoutRaid += PICKUP_HEALTH;

    // Generic code that must be executed for both kits
    helperGenericKitInteraction(interacted, interactorID, gctx);
}

/*Interaction: Heal Pad*/
// When you interact with the heal it starts to generate a heal kit
void interactionHealPad(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    helperGenericPadInteraction(interacted, gctx);    
}

/*Interaction: ammo Kit*/
// Interactor Ammo with touch
void interactionAmmoKit(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    // Only player interaction control
    TypeComponent* tipo = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
    if(tipo->type!=ENT_PLAYER)return;

    WeaponComponent* wcmp = gctx.entityManager.getComponentByID<WeaponComponent>(interactorID);
    if(!wcmp) return;
    
    //Sound Interaction
    gctx.audioManager.playSound(SOUND_SFX_PICKUP_AMMO, SOUNDS_SFX_AMBIENCE);
    gctx.audioManager.playSound(SOUND_VFX_LIAM_PICKUP_AMMO, SOUNDS_VOICE);
    //restore ammunition, 20% to all weapons
    cf::restoreAmmo(*wcmp);
    gctx.gameManager.timeWithoutRaid+=PICKUP_AMMO;

    // Generic code that must be executed for both kits
    helperGenericKitInteraction(interacted, interactorID, gctx);
}

/*Interaction: Ammo Pad*/
// When you interact with the Ammo it starts to generate a Ammo kit
void interactionAmmoPad(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    helperGenericPadInteraction(interacted, gctx); 
}

/*Interaction: Interaction Receive Card*/
// give the inventory of the interactor a card and eliminate that card
void interactionReceiveCard(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>( interactorID );
    cf::addKey(*playerInv, interacted.sharedKeyId, gctx);

    //Now kill the Card
    gctx.entityManager.markAsDead( interacted.getEntityID() );

    //Spawn a RAID
    gctx.gameManager.timeWithoutRaid+=PICKUP_CARD;
    // Send online
    const TypeComponent* pickupTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(interacted.getEntityID());
    const TypeComponent* interactorTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
    int serverInteractorId = -1;
    if(interactorTypeCmp) serverInteractorId = interactorTypeCmp->idInServer;
    if(pickupTypeCmp)
        gctx.networkClient.send(
            "PickupCollected" SEP
            +std::to_string(pickupTypeCmp->idInServer)
            +SEP+std::to_string(serverInteractorId)
        );

    if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            gctx.audioManager.playSound(SOUND_VFX_LIAM_PICKUP_CARD, SOUNDS_VOICE);
    }else{
            //Captain Complaining about doors
            //Pending to record
        }
}


/*Interaction: Interaction Receive Weapon*/
// give the inventory of the interactor a weapon depending type
void interactionReceiveWeapon(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    TypeComponent* tipo = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
    if(tipo->type != ENT_PLAYER) return;

    //Sound Interaction
    gctx.audioManager.playSound(SOUND_SFX_PICKUP_WEAPON, SOUNDS_SFX_AMBIENCE);

    WeaponComponent* weaponInv = gctx.entityManager.getComponentByID<WeaponComponent>( interactorID );
    WeaponID weaponID { (WeaponID)interacted.sharedKeyId };
    cf::addWeapon(*weaponInv, weaponID, &gctx, true);
    cf::equipWeapon(*weaponInv, gctx, weaponID);
    //Now kill the Weapon
    gctx.entityManager.markAsDead( interacted.getEntityID() );
}

/*Interaction: Interaction Open door with Card*/
// Open the door if you have the choosen card
void interactionOpenDoorWithCard(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>( interactorID );

    // Search the key in the player's inventory
    bool cardFound = false;
    for(int key : playerInv->keys)
        if(key == interacted.sharedKeyId) {
            cardFound = true;
            break;
        }

    if(cardFound)
    {
        //Sound Interaction
        gctx.audioManager.playSound(SOUND_SFX_CARD_SUCCESS, SOUNDS_SFX_AMBIENCE);
        interactionOpenDoor(interacted, interactorID, gctx);
    }
    else
    {
        //Sound Interaction Fail
        gctx.audioManager.playSound(SOUND_VFX_LIAM_FAIL_CARD_DOOR, SOUNDS_VOICE);
        gctx.audioManager.playSound(SOUND_SFX_CARD_FAIL, SOUNDS_SFX_AMBIENCE);

        // Throw info popup
        auto keyStr = std::to_string(interacted.sharedKeyId);
        gctx.hudManager.addPopup(
            "bottomSlot"
            , "cannotOpenDoor"
            , "You need the key "+keyStr+" to open"
            , "popupBackground"
            , "icon_card_unknown"
            , 1.5f
        );
    }

    //If you don't have keys sound must play
    if(playerInv->keys.size()==0){
        if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            gctx.audioManager.playSound(SOUND_VFX_LIAM_FAIL_CARD_DOOR, SOUNDS_VOICE);
        }else{
            //Captain Complaining about doors
            //Pending to record
        }
    }
}

/*Interaction: Vending Machine*/
//Creates a Kit if you buy it
void interactionVendingMachine(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>( interactorID );
    int points = playerInv->points;
    if(interacted.interactionData <= points)
    {
        // Check that the machine has not a kit already
        AIComponent* padAi = gctx.entityManager.getComponentByID<AIComponent>(interacted.idRemoteToInteractWith);
        if(padAi->hasKit == 2)
        {
            padAi->timeWithoutKit = 0;
            interactionRemoteEntityInteract(interacted, interactorID, gctx);

            // Send activation online to host (host spawns the health / ammo)
            auto* machineTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(interacted.getEntityID());
            if(machineTypeCmp)
            gctx.networkClient.send(
                "VendingMachineActivated" SEP
                +std::to_string(machineTypeCmp->idInServer)
                +SEP+std::to_string(interactorID)
            );

            // Substract credits from player's account
            cf::addCredits(*playerInv, -interacted.interactionData, gctx);

            //Sound of vending machine
            auto nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(interactorID);
            gctx.audioManager.play3DSound(SOUND_SFX_VENDING_MACHINE, SOUNDS_SFX_AMBIENCE, nodeCmp->node->getPosition());

        }

    }else{
        //Necesitamos exportar en fmod para los audios de alex faltandole dinero


        //gctx.audioManager.playSound(SOUND_SFX_NEGATIVE_INTERACTION);
        if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            //gctx.audioManager.playSound(SOUND_VFX_LIAM_FAIL_POINT_DOOR);
        }else{
            //Captain Complaining about doors
            //Pending to record
        }
    }
}

/*Interaction: Interaction Open door with Card*/
// Open the door if you have the points
void interactionOpenDoorWithPoints(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    auto playerInv = gctx.entityManager.getComponentByID<InventoryComponent>( interactorID );
    int points = playerInv->points;
    if(interacted.interactionData <= points)
    {
        
        //Sound Interaction
        gctx.audioManager.playSound(SOUND_SFX_DOOR_OPEN, SOUNDS_SFX_AMBIENCE);
        if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            gctx.audioManager.playSound(SOUND_VFX_LIAM_SUCCESS_POINT_DOOR, SOUNDS_VOICE);
        }else{
            //Captain Complaining about doors
            //Pending to record
        }
        interactionOpenDoor(interacted, interactorID, gctx);
        cf::addCredits(*playerInv, -interacted.interactionData, gctx);
    }else{
        //Sound Interaction
        gctx.audioManager.playSound(SOUND_SFX_NEGATIVE_INTERACTION, SOUNDS_SFX_AMBIENCE);
        if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            gctx.audioManager.playSound(SOUND_VFX_LIAM_FAIL_POINT_DOOR, SOUNDS_VOICE);
        }else{
            //Captain Complaining about doors
            //Pending to record
        }

        // Throw info popup
        auto creditsNeeded = std::to_string((int)interacted.interactionData - points);
        gctx.hudManager.addPopup(
            "bottomSlot"
            , "cannotOpenDoor"
            , "You need "+creditsNeeded+" more credits to open"
            , "popupBackground"
            , "icon_skull"
            , 1.5f
        );
    }
}

/*Interaction: Interaction Open door with Card*/
// Open the door if you have the points
void interactionOpenEnemyKillsDoor(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    int enemyKills = gctx.gameManager.killedMapEnemies;
    if(interacted.interactionData <= enemyKills)
    {
        //Sound Interaction
        gctx.audioManager.playSound(SOUND_SFX_DOOR_OPEN, SOUNDS_SFX_AMBIENCE);
        if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            gctx.audioManager.playSound(SOUND_VFX_LIAM_SUCCESS_POINT_DOOR, SOUNDS_VOICE);
        }else{
            //Captain Complaining about doors
            //Pending to record
        }
        //change the max time to respawn
        gctx.gameManager.maxtimeWithoutMaxEnemies = interacted.interactionData2;
        interactionOpenDoor(interacted, interactorID, gctx);
    }
    else
    {
        //Sound Interaction
        gctx.audioManager.playSound(SOUND_SFX_NEGATIVE_INTERACTION, SOUNDS_SFX_AMBIENCE);
        if(gctx.networkClient.isHost()){
            //Liam Complaining about doors
            gctx.audioManager.playSound(SOUND_VFX_LIAM_FAIL_POINT_DOOR, SOUNDS_VOICE);
        }else{
            //Captain Complaining about doors
            //Pending to record
        }

        // Throw info popup
        auto remainingEnemies = std::to_string((int)interacted.interactionData - enemyKills);
        gctx.hudManager.addPopup(
            "bottomSlot"
            , "cannotOpenDoor"
            , "Kill "+remainingEnemies+" more enemies to open"
            , "popupBackground"
            , "icon_skull"
            , 1.5f
        );
    }
}


/*HELPERS*/
void helperGenericKitInteraction(InteractableComponent& interacted, int interactorID, GameContext& gctx)
{
    //Interact with the pad so it knows it is empty
    AIComponent* padAI = gctx.entityManager.getComponentByID<AIComponent>( interacted.idRemoteToInteractWith );
    if(padAI) padAI->hasKit = 2;   

    //Now kill the kit
    gctx.entityManager.markAsDead( interacted.getEntityID() );

    // Send online
    const TypeComponent* pickupTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(interacted.getEntityID());
    if(pickupTypeCmp)
        gctx.networkClient.send(
            "PickupCollected" SEP
            +std::to_string(pickupTypeCmp->idInServer)
        );
}

void helperGenericPadInteraction(InteractableComponent& interacted, GameContext& gctx)
{
    AIComponent* padAI = gctx.entityManager.getComponentByID<AIComponent>( interacted.getEntityID() );
    padAI->hasKit = 0;  
}

void sendOnlineMessage(
    std::string msg,
    InteractableComponent& interacted,
    int interactorID,
    GameContext& gctx)
{
    // Unreachable entity is only used on purpose to avoid infinite broadcast loop
    if(interactorID > UNREACHABLE_ENTITY)
    {
        const TypeComponent* doorTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(interacted.getEntityID());
        const TypeComponent* interactorTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(interactorID);
        if(doorTypeCmp && interactorTypeCmp)
            gctx.networkClient.send(
                msg+SEP
                +std::to_string(doorTypeCmp->idInServer)
                +SEP+std::to_string(interactorTypeCmp->idInServer)
            );
    }
}

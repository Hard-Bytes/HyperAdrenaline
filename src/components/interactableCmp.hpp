#pragma once
#include "component.hpp"
#include "behaviourFunctions.hpp"

struct SpawnPoint
{
    Vector3f position;
    int patrolPath;
    int initWaypointIndex;
    int enemyType;
    int numEnemiesToSpawn;
    bool isForRaid;

    SpawnPoint(Vector3f pos, int p_patrol, int p_firstWaypt, int p_enemyType, bool p_isForRaid, int p_numEnemiesToSpawn = 1) :
        position(pos)
        , patrolPath(p_patrol)
        , initWaypointIndex(p_firstWaypt)
        , enemyType(p_enemyType)
        , numEnemiesToSpawn(p_numEnemiesToSpawn)
        , isForRaid(p_isForRaid)
    {}
};

struct InteractableComponent : public Component
{
    // Constructor
    explicit InteractableComponent(
        EntityID entID,
        InteractionFunction action=nullptr,
        int keyId=-1,
        int idToInteract=-1,
        int idToBeInteracted=-1,
        float interactionData=0
    );
    ~InteractableComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeInteractable; }

    // Action to do upon being interacted
    InteractionFunction interaction;

    std::vector<SpawnPoint> spawnPointsInteract;

    // Interaction data
    int sharedKeyId;            // Id of the key we are OR the key that opens us
    int idRemoteToInteractWith; // Id of remote to interact with
    int idRemoteToBeInteracted; // Our id to be interacted as a remote

    float interactionData;        // Multipurpose interaction data
    float interactionData2;       //Multipurpose interaction data
    //NECESITAMOS HACER UN COMPONENTE DE DATOS PARA LAS PUERTAS
    bool isRaidZone=false;

};

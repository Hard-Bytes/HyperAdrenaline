#include "gameManager.hpp"
#include "../system/aiBehaviours.hpp"
#include "../system/behaviourFunctions.hpp"
#include "../system/interactionBehaviours.hpp"
#include "../util/raidTimes.hpp"
#include "../components/weaponCmpFunc.hpp"
#include <hyperengine/engine/particles/generator.hpp>

GameManager::GameManager(
    EntityManager& entman
    , GraphicsEngine& graph
    , AudioManager& audio
    , HUDManager& hudMan
    , NetworkClient& netcli
    , SettingsManager& setman
    , SaveDataManager& saveMan
) 
:
entityManager(entman)
, graphicsEngine(graph)
, audioEngine(audio)
, hudManager(hudMan)
, networkClient(netcli)
, settingsManager(setman)
, saveDataManager(saveMan)
{}

GameManager::~GameManager()
{
    m_playerIDs.clear();
    m_lightList.clear();
}

/*Disable Node*/
// Makes a node invisible and turns off its collisions
void GameManager::disableNode(GraphicNode& node)
{
    graphicsEngine.disableAllCollisionsOf(&node);
    node.setVisible(false);
}

int GameManager::getLocalPlayerID() const noexcept
{
    if(!m_playerIDs.empty()) return m_playerIDs[0];
    return -1;
}

void GameManager::addPlayerNameToList(const std::string& name) noexcept 
{
    auto it = std::find(m_playerNames.begin(), m_playerNames.end(), name);
    if(it == m_playerNames.end()) m_playerNames.push_back(name); 
}

void GameManager::removePlayerNameFromList(const std::string& name) noexcept
{ 
    m_playerNames.erase(
        std::find(m_playerNames.begin(), m_playerNames.end(), name)
    ); 
}

/*Reset Game Data*/
// Resets various game parameters that must be reseted on new game load
void GameManager::resetGameData()
{
    this->m_playerIDs.clear();
    this->enemiesInGame = 0;
    this->killedMapEnemies = 0;
    this->inARaid = 0;
    this->timeWithoutRaid = 0;
    this->enemiesInARaid = 10;
    this->dangerLevel = 1;
    this->spawnArray.clear();
}

/*Clear Game*/
// Clears all entities
void GameManager::clearGame()
{
    auto& nodes { entityManager.getComponentVector<NodeComponent>() };
    for(auto& node : nodes)
        if(node.node)
            delete node.node;

    auto& ais { entityManager.getComponentVector<AIComponent>() };
    for(auto& aiCmp : ais)
        if(aiCmp.behaviourTree)
            delete aiCmp.behaviourTree;

    auto& ptcs { entityManager.getComponentVector<ParticleComponent>() };
    for(auto& ptcsCmp : ptcs)
    {
        if(ptcsCmp.generator)
            delete ptcsCmp.generator;
        if(ptcsCmp.generatorHitmark)
            delete ptcsCmp.generatorHitmark;
    }

    entityManager.clearAll();
    this->clearLightList();
    this->resetGameData();
}

/*Update Dead Entities*/
// Updates data for dead entities, but does not remove them (see EntityManger for that)
// This should ALWAYS be immediately followed by EntiyManager's updateDeadEntities
void GameManager::updateDeadEntities()
{
    auto& entities = entityManager.getEntities();
    for(auto& ent : entities)
        if(ent.isDead)
            this->destroyEntity(ent.getID());
}

/*Update Dynamic Lightning*/
// Update dynamic lightning. Enable dynamic lightning to use
// with gameManager.setDynamicLightning(true)
void GameManager::updateDynamicLightning(bool force) noexcept
{
    if(!graphicsEngine.getDynamicLightning()) return;

    // Update timer
    m_dynamicLightUpdateTimer -= graphicsEngine.getTimeDiffInSeconds();
    if(m_dynamicLightUpdateTimer > 0.0f && !force) return;
    m_dynamicLightUpdateTimer = m_dynamicLightUpdateInterval;

    // Get player position
    auto* playerNode { entityManager.getComponentByID<NodeComponent>(this->getLocalPlayerID()) };
    Vector3f playerPos { playerNode->node->getPosition() };

    // Check every light
    for(auto& light : m_lightList)
    {
        // Negative radius lights are always activated
        if(light.radius < 0)
        {
            graphicsEngine.setLightVisible(light.lightID, true);
            continue;
        }
        // Disable by distance (if the player is farther than the light's radius)
        float diff { (light.position - playerPos).length() };
        bool active { diff < light.radius };
        graphicsEngine.setLightVisible(light.lightID, active);
    }
}

/*Set Dynamic Lightning*/
// Specifies if dynamic lightning should be used
void GameManager::setDynamicLightning(bool isDynamic, bool updateLights) noexcept
{
    graphicsEngine.setDynamicLightning(isDynamic);
    if(m_lightList.empty()) return;

    if(isDynamic)
    {
        for(auto& lighti : m_lightList)
            graphicsEngine.setLightVisible(lighti.lightID, false);
        m_lightList.front().radius = 0;
        if(updateLights) this->updateDynamicLightning(true);
    }
    else
    {
        for(auto& lighti : m_lightList)
            graphicsEngine.setLightVisible(lighti.lightID, false);
        graphicsEngine.setLightVisible(m_lightList.front().lightID, true);
        m_lightList.front().radius = -1;
    }
}

/*Clear Light List*/
// Clear the light list
void GameManager::clearLightList() noexcept
{
    m_lightList.clear();
}

/*Create Player Entity*/
// Creates an entity of type Player (with camera)
int GameManager::createPlayerEntity(Vector3f pos, Vector3f initLookAt, PlayerLook look)
{
    EntityID entID = entityManager.createEntity();

    // Register as player
    m_playerIDs.push_back(entID);

    // Create its node
    GraphicNode* node = graphicsEngine.createCameraNode(pos, initLookAt);

    // Create collision box (for interactables, events and AoE)
    CollisionArea collider { CollisionShape::Box, {0.6, 1.8f, 0.6}, 0 };
    graphicsEngine.createNodeCollisions(
        node
        ,MASK MASK_PLAYER
        , false, false, false
        , collider
        , false // Don't use complex selector
    );

    // Create physic properties
    node->setPhysicOffset({0,-1.2,0});
    node->setSecondaryPhysicOffset({0,-1.2,0});
    graphicsEngine.createKinematicController(
        node
        , MASK MASK_MAP
        , 0.6f, 1.8f, 10.f, 0.3f
        , 25.0f // Gravity
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_PLAYER);
    entityManager.createComponent<InputComponent>(entID);
    HealthComponent& healthCmp = entityManager.createComponent<HealthComponent>(entID, 20);
    healthCmp.totalDeathCounter = 0.0f; //TODO:: Add HUD animation duration or something
    hudManager.setLifebarLengthAndCount(healthCmp.health, healthCmp.maxHealth);

    auto& wpnCmp { entityManager.createComponent<WeaponComponent>(entID) };
    auto unlockedWeapons { saveDataManager.loadObtainedWeapons() };
    for(auto value : unlockedWeapons)
        cf::addWeapon(wpnCmp, (WeaponID)value);

    auto& inv = entityManager.createComponent<InventoryComponent>(entID);
    hudManager.setCreditCount(inv.points);

    CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
        entID,             // ID
        Vector3f(1,2,1),   // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(10,17.f,10),     // Base Speed
        Vector3f(0,4.55f,0)       //OffSet
    );
    collisonCmp.moves = true;
    collisonCmp.collidesWithMap = true;
    collisonCmp.collidesWithEvents = true;
    collisonCmp.collidesWithAreaEffects = true;
    collisonCmp.previousPosition = pos;

    // partículas
    // auto& particleComponent = entityManager.createComponent<ParticleComponent>(entID);

    // hyen::ParticleGenerator::CInfo cInfo;
    // cInfo.maxParticles =        {100};
    // // cInfo.texturePath =         {"assets/particles/flare2.png"};
    // cInfo.texturePath =         {"assets/particles/particle.DDS"};
    // cInfo.origin =              {0,0,0};
    // cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    // // cInfo.gravity =             {0.0f, -9.81f, 0.0f};
    // cInfo.mainDir =             {0.0f,  0.0f, 0.0f};
    // cInfo.particlesPerSecond =  {100.0f};
    // cInfo.spreadFactor =        {0.5f};
    // cInfo.lifeSpan =            {1.0f};
    // cInfo.minParticleSize =     {0.15f};
    // cInfo.maxParticleSize =     {0.5f};
    // cInfo.shapeRadius =         {3.f};
    // cInfo.funcColor =           {&hyen::PGF::generateParticleColorsStandard};
    // cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};
    // cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionSoftInfluence};
    // // cInfo.funcPos =             {&hyen::PGF::generatePositionCameraTarget};
    // cInfo.funcPos =             {&hyen::PGF::generatePositionStatic};
    // cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};
    // // cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionCameraTarget};
    // particleComponent.generator = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };

    // Get online name and send connection
    networkClient.send(
        "PlayerSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+m_playerName
        +SEP+std::to_string(healthCmp.health)
        +SEP+std::to_string(wpnCmp.weapons[wpnCmp.index].id)
        +SEP+std::to_string(look)
    );

    // Return the ID
    return entID;
}

/*Create NPC Player Entity*/
// Creates an entity of type NPC Player (player controlled by network)
int GameManager::createNPCPlayerEntity(Vector3f pos, PlayerLook look)
{
    EntityID entID = entityManager.createEntity();

    // Register as player
    int playerHUDIndex { (int)m_playerIDs.size() - 1 };
    m_playerIDs.push_back(entID);

    std::string model { MODEL_CHARAC_LIAM };
    if(look == PL_Captain)
        model = MODEL_CHARAC_HANS;

    // Create its node
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        Vector3f(1,1,1),
        model
    );
    // Create collision box (for interactables, events and AoE)
    CollisionArea collider { CollisionShape::Box, {0.6, 1.6f, 0.6}, 0 };
    graphicsEngine.createNodeCollisions(
        node
        ,MASK MASK_PLAYER
        , false, false, false
        , collider
        , false // Don't use complex selector
    );
    // Create physic properties
    Vector3f offset { 0, 1.9, 0};
    node->setPhysicOffset({0,1.6,0});
    node->setSecondaryPhysicOffset({0,1.9,0});
    graphicsEngine.moveNodeCollisionsToSecondarySlot(node);

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    auto& typeCmp { entityManager.createComponent<TypeComponent>(entID, ENT_PLAYER) };
    typeCmp.style = look;
    HealthComponent& healthCmp { entityManager.createComponent<HealthComponent>(entID, 20) };
    healthCmp.totalDeathCounter = 0.0f; // TODO::Animation(?)

    AIComponent& aiCmp = entityManager.createComponent<AIComponent>(entID, USELESS, nullptr);
    aiCmp.lookAt = {0.5f, 0, 0.5f};

    auto& wpnCmp { entityManager.createComponent<WeaponComponent>(entID, Vector3f(-0.7f,2.8f,1.5f)) };
    wpnCmp.generalScale = {0.5, 0.5, 0.5};
    auto& invCmp { entityManager.createComponent<InventoryComponent>(entID) };

    // Add to HUD
    hudManager.setOtherPlayerInfoStyle(playerHUDIndex, look);
    hudManager.setOtherPlayerInfoVisible(playerHUDIndex, true);
    hudManager.setOtherPlayerLifebar(playerHUDIndex, healthCmp.health, healthCmp.maxHealth);
    hudManager.setOtherPlayerCredits(playerHUDIndex, invCmp.points);

    CollisionComponent& collisionCmp = entityManager.createComponent<CollisionComponent>(
        entID,             // ID
        Vector3f(1,2,1),   // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(10,0.0f,10)     // Base Speed
    );
    collisionCmp.moves = true;
    //collisionCmp.collidesWithMap = true;
    collisionCmp.offset = offset;

    // Return the ID
    return entID;
}

/*Create Camera Entity*/
// Creates an entity of type Camera (for menus)
int GameManager::createCameraEntity(Vector3f pos, Vector3f initLookAt)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    GraphicNode* node = graphicsEngine.createCameraNode(pos, initLookAt);
    entityManager.createComponent<NodeComponent>(entID, node);

    entityManager.createComponent<BehaviourComponent>(entID, behMenuCameraRotation);

    // Return the ID
    return entID;
}

/*Create Enemy (Factory)*/
// A factory that creates an enemy given its type and generic data
int GameManager::createEnemyEntity(
    int enemyType,
    Vector3f pos,
    Vector3f size,
    Vector3f rot,
    int path,
    int initWaypointIndex,
    bool isRaidEnemy)
{
    int id;
    switch (enemyType)
    {
    case 4: //Shooting ++
        id = createShootingEnemyEntity
        (
            pos,
            size,
            rot,
            path,
            initWaypointIndex,
            isRaidEnemy,
            true // Long range
        );
        break;
    case 3: //Dummy
        id = createDummyEntity
        (
            pos,
            size,
            rot
        );
    break;
    case 2: //Kamikaze
        id = createKamikazeEnemyEntity
        (
            pos,
            size,
            rot,
            8.f,
            path,
            initWaypointIndex,
            isRaidEnemy
        );
    break;
    case 1: //Shooter
        id = createShootingEnemyEntity
        (
            pos,
            size,
            rot,
            path,
            initWaypointIndex,
            isRaidEnemy,
            false // Long range
        );
    break;

    default:
    case 0: //Melee
        id = createMeleeEnemyEntity
        (
            pos,
            size,
            rot,
            path,
            initWaypointIndex,
            isRaidEnemy
        );
    break;
    }

    return id;
}

/*
int createEnemyEntityParam(
    int enemyType
    ,Vector3f pos
    ,Vector3f size
    ,Vector3f rot
    ,int path
    ,int initWaypointIndex
    ,bool isRaidEnemy)
{
    // PARAMS
    bool isAnimated { true }; // { false } { true } { true } { true } // TODO:: Use
    bool addsToEnemyCount { true }; // { false } { true } { true } { true }
    std::string model { MODEL_CHARAC_ENEMY_MELEE }; // MODEL_CHARAC_ENEMY_DUMMY, MODEL_CHARAC_ENEMY_MELEE, MODEL_CHARAC_ENEMY_SHOOTER, MODEL_CHARAC_ENEMY_KAMIKAZE
    Vector3f hitboxOffset {0,1.5,-0.2}; // {0,0,0} {0,1.5,-0.2}
    Vector3f hitboxSecondaryOffset {0,1.2,0}; // {0,0,0} {0,1.2,0}
    Vector3f hitboxSecondarySize {0.5, 1.8, 0.5}; // Default/useless {0.5, 1.8, 0.5} {0.5, 1.8, 0.5} {0.5, 1.8, 0.5}
    bool hasKinematicController { true }; // Dummy is false
        float controllerCapsuleRadius { 0.5f };
        float controllerCapsuleHeight { 2.4f };
        float controllerJumpForce     { 2.0f };
        float controllerStepHeight    { 0.9f };
        float controllerGravityForce  { 25.f };
    float lifePoints {20}; // 1, 20, 15, 15
    float deathAnimationTime {1.f}; // 0, 1, 1, 1
    int pointReward {50}; // 10, 50, 50, 50
    float speed {40}; // 1, 40, 64, 80
    bool moves { true }; // Dummy is false
    InteractionFunction specialAction { nullptr }; // interactionKamikazeSelfDestruct
    float explosionDamage { 8.f};
    //float attackDamage { .f};
    bool isSolid { false }; // Dummy is true
    bool usesComplexSelector { false }; // Dummy is true
    bool hasAI { true }; // Dummy is false
        bool usesSteeringSeparation { true }; // True for melee and kami, false for ranged
        bool defaultSteeringIsSeek { false }; // True for kami, false for melee and ranged
        float shootingRange { 25.0f }; // 25 in normal ranged, 100 in super ranged
    bool hasWeapon { false }; // Shooter is true
        Vector3f weaponOffset { 0.5f,1.f,0.5f }; // Only if weapon exists
    bool affectedByStatus { false }; // Dummy is false
    bool affectedByAreas { true }; // Everyone is true

    EntityID entID = entityManager.createEntity();
    if(addsToEnemyCount) enemiesInGame++;

    // Create its node
    CollisionArea collData { CollisionShape::Box, hitboxSecondarySize, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        model
    );
    graphicsEngine.createNodeCollisions(
        node
        ,MASK MASK_ENEMY
        ,isSolid     // Is solid
        ,false       // Triggers an area effect when touched
        ,false       // Triggers an event when touched
        ,collData
        ,usesComplexSelector
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);
    node->setPhysicOffset(hitboxOffset);
    node->setSecondaryPhysicOffset(hitboxSecondaryOffset);
    if(hasKinematicController)
    {
        graphicsEngine.createKinematicController(
            node
            , MASK MASK_MAP
            , controllerCapsuleRadius
            , controllerCapsuleHeight
            , controllerJumpForce
            , controllerStepHeight
            , controllerGravityForce
        );
    }

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ENEMY);
    entityManager.createComponent<InteractableComponent>(entID, specialAction, pointReward, 0, 0, explosionDamage);
    auto& healthCmp { entityManager.createComponent<HealthComponent>(entID, lifePoints) };
    healthCmp.totalDeathCounter = deathAnimationTime;

    // Status component
    if(affectedByStatus) entityManager.createComponent<StatusComponent>(entID);

    // Collision Component
    CollisionComponent& collisonCmp { entityManager.createComponent<CollisionComponent>(
        entID,                 // ID
        Vector3f{1,1,1},       // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(speed, 0.125f, speed) // Base Speed
    ) };
    collisonCmp.moves = moves;
    collisonCmp.collidesWithMap = moves;
    collisonCmp.collidesWithAreaEffects = affectedByAreas;
    collisonCmp.useSteering = true;

    // AI Component
    if(hasAI)
    {
        TREE_TYPE behTree;
        switch(enemyType)
        {
            default:
            case 0:
                if(isRaidEnemy) behTree = RAID_MELEE_TREE;
                else            behTree = MELEE_TREE;
                break;
            case 1:
            case 4:
                if(isRaidEnemy) behTree = RAID_SHOOTING_TREE;
                else            behTree = SHOOTING_TREE;
                break;
            case 2:
                behTree = KAMIKAZE_TREE;
                break;
        }
        AIComponent& aiCmp { entityManager.createComponent<AIComponent>(entID, behTree, aiPatroller) };
        aiCmp.pathIndex = path;
        aiCmp.currentPatrolPath = initWaypointIndex;
        aiCmp.isRaidEnemy = isRaidEnemy;
        aiCmp.shootingRange = shootingRange;
        if(defaultSteeringIsSeek)
        {
            aiCmp.steeringBehaviours.clear();
            aiCmp.steeringBehaviours.push_back(SB_SEEK);
        }
        if(usesSteeringSeparation)
        {
            aiCmp.steeringBehaviours.push_back(SB_SEPARATION);
        }
    }

    // Create weapon if needed
    if(hasWeapon)
    {
        WeaponComponent& weaponCmp = entityManager.createComponent<WeaponComponent>(entID);
        cf::addWeapon(weaponCmp, W_ENEMY);
        weaponCmp.weapons.back().offset = weaponOffset;
    }

    // Send online
    if(networkClient.isHost())
    networkClient.send(
        "EnemySpawned" SEP
        +std::to_string(entID)
        +SEP+std::to_string(enemyType)
        +SEP+std::to_string(healthCmp.health)
        +SEP+std::to_string(isRaidEnemy)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rot.toNetworkString()
    );

    // Return the ID
    return entID;
}
*/

/*Create Dummy Entity*/
// Creates an entity of type Dummy (useless black box)
int GameManager::createDummyEntity(Vector3f pos, Vector3f size, Vector3f rot)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, {0.5, 3.5, 0.5}, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        MODEL_CHARAC_ENEMY_DUMMY
    );
    graphicsEngine.createNodeCollisions(
        node
        ,MASK MASK_ENEMY
        ,true  // Is solid
        ,false // Triggers an area effect when touched
        ,false // Triggers an event when touched
        ,collData // Default because it will be ignored
        ,true   // Use complex selector
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ENEMY);
    HealthComponent& healthCmp = entityManager.createComponent<HealthComponent>(entID, 1);
    healthCmp.totalDeathCounter = 0.0f;
    CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
        entID,     // ID
        size,               // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(1,1,1)     // Base Speed
        
    );
    collisonCmp.moves = false;
    collisonCmp.collidesWithAreaEffects = true;

    //Interactuable Comp -> points in death
    entityManager.createComponent<InteractableComponent>(entID,nullptr,10);

    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addFireParticleSystem(particleComponent);
    particleComponent.offset = {0,1.0,0};
    particleComponent.priorityLevel = 1;
    
    if(networkClient.isHost())
    networkClient.send(
        "EnemySpawned" SEP
        +std::to_string(entID)
        +SEP+std::to_string(3)
        +SEP+std::to_string(healthCmp.health)
        +SEP+std::to_string(false)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rot.toNetworkString()
    );

    // Return the ID
    return entID;
}

/*Create Melee Enemy Entity*/
// Creates an entity of type Melee Enemy (harmful enemy)
int GameManager::createMeleeEnemyEntity(
    Vector3f pos,
    Vector3f size,
    Vector3f rot,
    int path,
    int initWaypointIndex,
    bool isRaidEnemy)
{
    EntityID entID = entityManager.createEntity();
    enemiesInGame++;

    // Create its node
    CollisionArea collData { CollisionShape::Box, {0.5, 1.8, 0.5}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos,
        size,
        MODEL_CHARAC_ENEMY_MELEE,
        0.041666f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_ENEMY
        , false  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    node->setRotation(rot);
    //node->setPhysicRotation(rot);
    node->setPhysicOffset({0,1.5,-0.2});
    node->setSecondaryPhysicOffset({0,1.2,0});
    node->playAnimation("walk", true);

    bool collWithMap { true };
    if(networkClient.isHost())
    {
        graphicsEngine.createKinematicController(
            node
            , MASK MASK_MAP
            , 0.5f, 2.4f, 2.f, 0.9f
            , 9.8f // Gravity
        );
    }
    else
    {
        graphicsEngine.moveNodeCollisionsToSecondarySlot(node);
        collWithMap = false;
    }

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ENEMY);
    HealthComponent& healthCmp = entityManager.createComponent<HealthComponent>(entID, 20);
    if(!node->hasAnimation()) healthCmp.totalDeathCounter = 0.0f;

    auto nodeScale= node->getScale();
    CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
        entID,      // ID
        nodeScale,                // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(40,0.125f,40) // Base Speed
    );
    collisonCmp.moves = true;
    collisonCmp.collidesWithMap = collWithMap;
    collisonCmp.collidesWithAreaEffects = true;
    collisonCmp.useSteering = true;


    //Interactuable Comp -> points in death
    entityManager.createComponent<InteractableComponent>(entID,nullptr,50);

    TREE_TYPE raid;
    if(isRaidEnemy)
        raid = RAID_MELEE_TREE;
    else
        raid = MELEE_TREE;        

    AIComponent& aiCmp = entityManager.createComponent<AIComponent>(entID,raid, aiPatroller, Vector3f(),Vector3f(),path);
    aiCmp.currentPatrolPath = initWaypointIndex;
    aiCmp.isRaidEnemy = isRaidEnemy;
    aiCmp.steeringBehaviours.push_back(SB_SEPARATION);

    entityManager.createComponent<StatusComponent>(entID);

    // Particles
    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addFireParticleSystem(particleComponent);
    particleComponent.offset = {0,1.0,0};
    particleComponent.priorityLevel = 1;

    if(networkClient.isHost())
    networkClient.send(
        "EnemySpawned" SEP
        +std::to_string(entID)
        +SEP+std::to_string(0)
        +SEP+std::to_string(healthCmp.health)
        +SEP+std::to_string(isRaidEnemy)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rot.toNetworkString()
    );

    // Return the ID
    return entID;
}

/*Create Shooting Enemy Entity*/
// Creates an shooting entity of type Enemy (harmful enemy)
int GameManager::createShootingEnemyEntity(
    Vector3f pos,
    Vector3f size,
    Vector3f rot,
    int path,
    int initWaypointIndex,
    bool isRaidEnemy
    ,bool isLongRange)
{
    EntityID entID = entityManager.createEntity();
    enemiesInGame++;

    // Create its node
    CollisionArea collData { CollisionShape::Box, {0.5, 1.8, 0.5}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos,
        size,
        MODEL_CHARAC_ENEMY_SHOOTER,
        0.041666f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_ENEMY
        , false  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    //node->setPhysicRotation(rot);
    node->setPhysicOffset({0,1.5,-0.2});
    node->setSecondaryPhysicOffset({0,1.2,0});
    node->playAnimation("walk", true);

    bool collWithMap { true };
    if(networkClient.isHost())
    {
        graphicsEngine.createKinematicController(
            node
            , MASK MASK_MAP
            , 0.5f, 2.4f, 2.f, 0.9f
            , 9.8f // Gravity
        );
    }
    else
    {
        graphicsEngine.moveNodeCollisionsToSecondarySlot(node);
        collWithMap = false;
    }

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ENEMY);
    HealthComponent& healthCmp = entityManager.createComponent<HealthComponent>(entID, 15);
    if(!node->hasAnimation()) healthCmp.totalDeathCounter = 0.0f;
    WeaponComponent& weaponCmp = entityManager.createComponent<WeaponComponent>(entID);
    cf::addWeapon(weaponCmp, W_ENEMY);
    weaponCmp.weapons.back().offset = Vector3f(0,2.f,0);

    auto nodeScale= node->getScale();
    CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
        entID,      // ID
        nodeScale,   // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(64,0.125f,48) // Base Speed
    );
    collisonCmp.moves = true;
    collisonCmp.collidesWithMap = collWithMap;
    collisonCmp.collidesWithAreaEffects = true;
    collisonCmp.useSteering = true;

    //Interactuable Comp -> points in death
    entityManager.createComponent<InteractableComponent>(entID,nullptr,50);

    TREE_TYPE raid;
    if(isRaidEnemy)
        raid = RAID_SHOOTING_TREE;
    else
        raid = SHOOTING_TREE; 
    
    AIComponent& aiCmp = entityManager.createComponent<AIComponent>(entID,raid, aiPatroller, Vector3f(),Vector3f(),path);
    aiCmp.currentPatrolPath = initWaypointIndex;
    aiCmp.isRaidEnemy = isRaidEnemy;
    if(isLongRange) aiCmp.shootingRange = 100.f;

    entityManager.createComponent<StatusComponent>(entID);
    
    // Particles
    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addFireParticleSystem(particleComponent);
    particleComponent.offset = {0,1.0,0};
    particleComponent.priorityLevel = 1;

    if(networkClient.isHost())
    networkClient.send(
        "EnemySpawned" SEP
        +std::to_string(entID)
        +SEP+std::to_string(1)
        +SEP+std::to_string(healthCmp.health)
        +SEP+std::to_string(isRaidEnemy)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rot.toNetworkString()
    );

    // Return the ID
    return entID;

}

/*Create Melee Enemy Entity*/
// Creates an entity of type Melee Enemy (harmful enemy)
int GameManager::createKamikazeEnemyEntity(
    Vector3f pos,
    Vector3f size,
    Vector3f rot,
    float damage,
    int path,
    int initWaypointIndex,
    bool isRaidEnemy)
{
    EntityID entID = entityManager.createEntity();
    enemiesInGame++;

    // Create its node
    CollisionArea collData { CollisionShape::Box, {0.5, 1.8, 0.5}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos,
        size,
        MODEL_CHARAC_ENEMY_KAMIKAZE,
        0.041666f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_ENEMY
        , false  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    //node->setPhysicRotation(rot);
    node->setPhysicOffset({0,1.5,-0.2});
    node->setSecondaryPhysicOffset({0,1.2,0});
    node->playAnimation("walk", true);

    bool collWithMap { true };
    if(networkClient.isHost())
    {
        graphicsEngine.createKinematicController(
            node
            , MASK MASK_MAP
            , 0.5f, 2.4f, 2.f, 0.9f
            , 9.8f // Gravity
        );
    }
    else
    {
        graphicsEngine.moveNodeCollisionsToSecondarySlot(node);
        collWithMap = false;
    }

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ENEMY);
    entityManager.createComponent<InteractableComponent>(entID, interactionKamikazeSelfDestruct,50,0,0,damage);
    HealthComponent& healthCmp { entityManager.createComponent<HealthComponent>(entID, 15) };
    if(!node->hasAnimation()) healthCmp.totalDeathCounter = 0.0f;

    auto nodeScale= node->getScale();
    CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
        entID,      // ID
        nodeScale,     // Size
        Vector3f(0,0,0),       // Gravity
        Vector3f(80,0.125f,80) // Base Speed
    );
    collisonCmp.moves = true;
    collisonCmp.collidesWithMap = collWithMap;
    collisonCmp.collidesWithAreaEffects = true;
    collisonCmp.useSteering = true;
    
    AIComponent& aiCmp = entityManager.createComponent<AIComponent>(entID,KAMIKAZE_TREE, aiPatroller, Vector3f(),Vector3f(),path);
    aiCmp.currentPatrolPath = initWaypointIndex;
    aiCmp.isRaidEnemy = isRaidEnemy;
    aiCmp.steeringBehaviours.clear();
    aiCmp.steeringBehaviours.push_back(SB_SEPARATION);
    aiCmp.steeringBehaviours.push_back(SB_SEEK);

    entityManager.createComponent<StatusComponent>(entID);

    // Particles
    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addFireParticleSystem(particleComponent);
    particleComponent.offset = {0,1.0,0};
    particleComponent.priorityLevel = 1;

    if(networkClient.isHost())
    networkClient.send(
        "EnemySpawned" SEP
        +std::to_string(entID)
        +SEP+std::to_string(2)
        +SEP+std::to_string(healthCmp.health)
        +SEP+std::to_string(isRaidEnemy)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rot.toNetworkString()
    );

    // Return the ID
    return entID;
}

/*Create Exit Entity*/
// Creates an entity of type "Level Exit Trigger Box"
int GameManager::createExitEntity(Vector3f pos, Vector3f size)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        {0,0,0},
        Vector3f(0.341, 0.075, 0.306)
        ,false
    );
    graphicsEngine.createNodeCollisions(
        node
        ,MASK MASK_USELESS
        ,false   // Is solid
        ,false   // Triggers an area effect when touched
        ,true    // Triggers an event when touched
        ,collData
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_EVENTAREA);
    entityManager.createComponent<InteractableComponent>(entID, interactionLevelFinished);

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(ME_LevelExit)
    );

    // Return the ID
    return entID;
}


/*Create Solid Obstacle Entity*/
// Creates an entity of type "Solid", like a door
int GameManager::createSolidEntity(Vector3f pos, Vector3f size)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos
        ,{0,0,0}
        ,Vector3f(0, 0.075 , 0.42)  // Color
        ,false
    );
    graphicsEngine.createNodeCollisions(
        node
        ,MASK MASK_MAP
        ,true   // Is solid
        ,false  // Triggers an area effect when touched
        ,false   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_OBSTACLE);

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(ME_Solid)
    );

    // Return the ID
    return entID;
}

/*Create Killing Zone Entity*/
// Creates an entity of type "Killing Zone Event Trigger", wich kills entities on touch
int GameManager::createKillingZoneEntity(Vector3f pos, Vector3f size)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        Vector3f(0.733, 0.075, 0.306)  // Color
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_USELESS
        , false   // Is solid
        , true    // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_EFFECTAREA);
    entityManager.createComponent<InteractableComponent>(entID, interactionInKillingArea);

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(ME_KillingArea)
    );

    // Return the ID
    return entID;
}

/*Create Damaging Zone Entity*/
// Creates an entity of type "Damaging Zone Event Trigger", wich damages entities on touch
int GameManager::createDamagingZoneEntity(Vector3f pos, Vector3f size, float damagePerSecond)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        Vector3f(0.576, 0.039, 0.11)  // Color
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_USELESS
        , false  // Is solid
        , true   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_EFFECTAREA);
    entityManager.createComponent<InteractableComponent>(entID, interactionInDamagingArea,0,0,0,damagePerSecond);

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(ME_DamagingArea)
        +SEP SEP SEP SEP+std::to_string(damagePerSecond)
    );

    // Return the ID
    return entID;
}

/*Create Damaging Zone Per Time Entity*/
// Creates an entity of type "Damaging Zone Event Trigger", witch damages entities on touch and disapear after the time specified
int GameManager::createDamagingZonePerTimeEntity(Vector3f pos, Vector3f size,Vector3f rotate,Vector3f speed, float damagePerSecond, float time, bool collides, std::string model, std::string sound,EntityType eType)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    GraphicNode* node;
    CollisionArea collData { CollisionShape::Box, size, 0 };
    if(model.empty())
    {
        node = graphicsEngine.createNode(
            pos,
            size,
            Vector3f(0.576, 0.039, 0.11)  // Color
        );
    }
    else 
    {
        node = graphicsEngine.createNode(
            pos,
            size,
            model
        );
    }
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_USELESS
        , false  // Is solid
        , true   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    node->setRotation(rotate);
    node->setPhysicRotation(rotate);

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, eType);
    entityManager.createComponent<InteractableComponent>(entID, interactionInDamagingArea,0,0,0,damagePerSecond);
    
    if(collides)
    {
        BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behKillingMyselfByTimeAndCollision);
        behCmp.sound = sound;
        behCmp.initialPos = {time,0,0};
    }
    else
    {
        BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behKillingMyselfByTime);
        behCmp.sound = sound;
        behCmp.initialPos = {time,0,0};
    }

    if(sound.size()>0)
    {
        audioEngine.setSoundParameter(sound,"isFliying",1, SOUNDS_SFX_COMBAT); 
        audioEngine.playSound(sound, SOUNDS_SFX_COMBAT);
    }

    if(speed.x!=0 || speed.y!=0 || speed.z!=0 ){
        CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
            entID,      // ID
            size,                // Size
            Vector3f(0,0,0),   // Gravity
            Vector3f(1,1,1)// Base Speed
        );
        collisonCmp.speed=speed;
        collisonCmp.moves = true;
    }

    // Return the ID
    return entID;
}

/*Create ExpandableDamaging Zone Per Time Entity*/
// It creates a normal damaging zone that expands in time
int GameManager::createExpandableDamagingZonePerTimeEntity(Vector3f pos, Vector3f size,Vector3f rotate,Vector3f speed, float damagePerSecond, float time, std::string model, std::string sound, EntityType eType)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    GraphicNode* node;
    CollisionArea collData { CollisionShape::Box, size, 0 };
    if(model.empty())
    {
        node = graphicsEngine.createNode(
            pos,
            {0,0,0},
            Vector3f(0.576, 0.039, 0.11)  // Color
        );
    }
    else 
    {
        node = graphicsEngine.createNode(
            pos,
            {0,0,0},
            model
        );
    }
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_USELESS
        , false   // Is solid
        , true    // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    node->setRotation(rotate);
    node->setPhysicRotation(rotate);

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, eType);
    InteractableComponent& intCmp = entityManager.createComponent<InteractableComponent>(entID, interactionInDamagingArea,0,0,0,damagePerSecond);
    intCmp.interactionData2=damagePerSecond;
    {
        BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behScalateMyselfAndDieByTime);
        behCmp.sound = sound;
        behCmp.initialPos = {time,0,0};
    }

    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addExplosionParticleSystem(particleComponent, size);
    particleComponent.priorityLevel = 1;

    if(sound.size()>0)
    {
        audioEngine.setSoundParameter(sound,"isFliying",1, SOUNDS_SFX_COMBAT); 
        audioEngine.playSound(sound, SOUNDS_SFX_COMBAT);
    }

    if(speed.x!=0 || speed.y!=0 || speed.z!=0 ){
        CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
            entID,      // ID
            size,                // Size
            Vector3f(0,0,0),   // Gravity
            Vector3f(1,1,1)// Base Speed
        );
        collisonCmp.speed=speed;
        collisonCmp.moves = true;
    }

    // Return the ID
    return entID;
}

/*Create Explosion Zone Per Time Entity*/
// Creates an entity that explode when touch and enemy or the map
int GameManager::createExplosionMisilEntity(Vector3f pos, Vector3f size,Vector3f rotate,Vector3f speed, float damage, std::string model, std::string bulletSound, std::string explosionSound)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size*0.8f, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        model
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_USELESS
        , false  // Is solid
        , true   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    node->setRotation(rotate);
    node->setPhysicRotation(rotate);

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ATTACKAREA);
    entityManager.createComponent<InteractableComponent>(entID, interactionExplosionArea,0,0,0,damage);
    BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behTriggerMyselfByCollision);
    behCmp.sound = bulletSound;
    behCmp.soundExplosion = explosionSound;

    if(bulletSound.size()>0)
    {
        audioEngine.setSoundParameter(bulletSound,"isFliying",1, SOUNDS_SFX_COMBAT); 
        audioEngine.playSound(bulletSound, SOUNDS_SFX_COMBAT);
    }
    if(speed.x!=0 || speed.y!=0 || speed.z!=0 ){

        auto sizenode= node->getScale();
        CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
            entID,      // ID
            sizenode,                // Size
            Vector3f(0,0,0),   // Gravity
            Vector3f(1,1,1)// Base Speed
        );
        collisonCmp.speed=speed;
        collisonCmp.moves = true;
    }

    // Return the ID
    return entID;
}

/*Really dirty things*/
int GameManager::createEnemyBullet(Vector3f pos, Vector3f size,Vector3f rotate, Vector3f speed, float damage, float time, std::string sound)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        Vector3f(1,0,1)  // Color
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_USELESS
        , false  // Is solid
        , false   // Triggers an area effect when touched
        , true   // Triggers an event when touched
        , collData
    );

    node->setRotation(rotate);
    node->setPhysicRotation(rotate);

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, ENT_ENEMYATTACKAREA);
    entityManager.createComponent<InteractableComponent>(entID, interactionExplosionArea,0,0,0,damage);
    
    BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behKillingMyselfByTimeAndCollision);
    behCmp.sound = sound;
    behCmp.initialPos = {time,0,0};

    if(speed.x!=0 || speed.y!=0 || speed.z!=0 ){
        CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
            entID,      // ID
            size,                // Size
            Vector3f(0,-0.01,0),   // Gravity
            Vector3f(1,1,1)// Base Speed
        );
        collisonCmp.speed=speed;
        collisonCmp.moves = true;
    }
    
    if(networkClient.isHost())
    networkClient.send(
        "EnemyBulletShooted" SEP
        +pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rotate.toNetworkString()
        +SEP+speed.toNetworkString()
        +SEP+std::to_string(damage)
        +SEP+std::to_string(time)
        +SEP+sound
    );

    // Return the ID
    return entID;
}

/*Create Interactuable Remote Entity*/
// Creates an entity that acces to other interact and make it happens
int GameManager::createInteractuableRemoteEntity(Vector3f pos, Vector3f size, int idToInteract)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        Vector3f(0.04, 0.04, 0.04)  // Color
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE
        , true  // Is solid
        , false   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<InteractableComponent>(entID, interactionRemoteInteract, -1, idToInteract);
    entityManager.createComponent<TypeComponent>(entID, ENT_OTHER);


    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(ME_Button)
        +SEP SEP+std::to_string(idToInteract)
    );

    // Return the ID
    return entID;
}

/*Create Interactuable Remote Entity*/
// Creates an entity that acces to other interact and make it happens
int GameManager::createInteractuableRemoteWithCardEntity(Vector3f pos, Vector3f size, int cardID, int idToInteract, Vector3f rotate )
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    std::string lectorObj= MODEL_OBJECT_CARD_LECTOR;
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        lectorObj  // Model
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE
        , true  // Is solid
        , false   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rotate);
    node->setPhysicRotation(rotate);

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_OTHER);
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<InteractableComponent>(entID, interactionRemoteWithCardInteract, cardID, idToInteract);

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+rotate.toNetworkString()
        +SEP+std::to_string(ME_ButtonKeyControlled)
        +SEP+std::to_string(cardID)
        +SEP+std::to_string(idToInteract)
    );

    // Return the ID
    return entID;
}

/*Create Interactable Door Entity*/
// Creates an entity of type "Solid", like a door
int GameManager::createInteractuableDoorEntity(Vector3f pos, Vector3f size, Vector3f rot, bool isRemote, int remoteId, std::vector<SpawnPoint> spawnPointsDoor)
{
    EntityID entID = entityManager.createEntity();
    //Check if is remote
    int mask=0;
    if(isRemote)
        mask= MASK MASK_MAP;
    else
        mask= MASK MASK_INTERACTABLE | MASK MASK_MAP;
    // Create its node
    std::string doorObj { MODEL_OBJECT_DOOR_CARD };
    CollisionArea collData { CollisionShape::Box, {2.7f, 2.8f, 0.9f}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos
        ,size
        ,doorObj
        ,0.025f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        ,mask
        , true  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);
    node->setPhysicOffset({0.1f, 2.85f, 0.25f});
    node->playAnimation("closed-idle", false);

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_UNLOCKEDDOOR);
    entityManager.createComponent<NodeComponent>(entID, node);
    HealthComponent& healthCmp { entityManager.createComponent<HealthComponent>(entID, 1) };
    if(node->hasAnimation())
        healthCmp.totalDeathCounter = 1.0f;
    else
        healthCmp.totalDeathCounter = 0.0f;

    InteractableComponent& intCmp = entityManager.createComponent<InteractableComponent>(entID, interactionOpenDoor, -1, -1, remoteId);
    intCmp.spawnPointsInteract = spawnPointsDoor;

    if(networkClient.isHost())
    networkClient.send(
        "DoorSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+rot.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+std::to_string(isRemote ? D_RemoteDoor : D_InteractableDoor)
        +(remoteId<0 ? "" : (SEP SEP+std::to_string(remoteId))) // Only add the remote id if it's valid
    );

    // Return the ID
    return entID;
}

/*Create Door With Card Entity*/
// Creates a door that you can open with a card
int GameManager::createInteractuableDoorWithCardEntity(Vector3f pos, Vector3f size, Vector3f rot, int cardID, int remoteId, bool isRaidZone, float respawnTime ,std::vector<SpawnPoint> spawnPointsDoor)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    std::string doorObj { MODEL_OBJECT_DOOR_CARD };
    CollisionArea collData { CollisionShape::Box, {2.7f, 2.8f, 0.9f}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos,
        size,
        doorObj,
        0.025f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE | MASK MASK_MAP
        , true  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);
    node->setPhysicOffset({0.1f, 2.85f, 0.25f});
    node->playAnimation("closed-idle", false);

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_KEYDOOR);
    entityManager.createComponent<NodeComponent>(entID, node);
    HealthComponent& healthCmp { entityManager.createComponent<HealthComponent>(entID, 1) };
    if(node->hasAnimation())
        healthCmp.totalDeathCounter = 1.0f;
    else
        healthCmp.totalDeathCounter = 0.0f;

    InteractableComponent& intCmp=entityManager.createComponent<InteractableComponent>(entID, interactionOpenDoorWithCard, cardID, -1, remoteId);
    intCmp.spawnPointsInteract = spawnPointsDoor;
    intCmp.interactionData2 = respawnTime;
    intCmp.isRaidZone = isRaidZone;

    if(networkClient.isHost())
    networkClient.send(
        "DoorSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+rot.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+std::to_string(D_KeyDoor)
        +SEP+std::to_string(cardID)
        +SEP+std::to_string(remoteId)
    );

    // Return the ID
    return entID;
}

/*Create Door With Card Entity*/
// Creates a door that you can open with a card
int GameManager::createInteractuableDoorWithPointsEntity(Vector3f pos, Vector3f size, Vector3f rot, int doorPoints,std::vector<SpawnPoint> spawnPointsDoor)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    std::string doorObj { MODEL_OBJECT_DOOR };
    CollisionArea collData { CollisionShape::Box, {2.7f, 2.8f, 0.9f}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos,
        size,
        doorObj,
        0.025f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE | MASK MASK_MAP
        , true  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);
    node->setPhysicOffset({0.1f, 2.85f, 0.25f});
    node->playAnimation("closed-idle", false);

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_CREDITSDOOR);
    entityManager.createComponent<NodeComponent>(entID, node);
    HealthComponent& healthCmp { entityManager.createComponent<HealthComponent>(entID, 1) };
    if(node->hasAnimation())
        healthCmp.totalDeathCounter = 1.0f;
    else
        healthCmp.totalDeathCounter = 0.0f;
    InteractableComponent& intCmp = entityManager.createComponent<InteractableComponent>(entID, interactionOpenDoorWithPoints, -1,-1,-1, doorPoints);
    intCmp.spawnPointsInteract = spawnPointsDoor;

    if(networkClient.isHost())
    networkClient.send(
        "DoorSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+rot.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+std::to_string(D_CreditsDoor)
        +SEP+std::to_string(doorPoints)
    );

    // Return the ID
    return entID;
}

/*Create Interactable Door Entity*/
// Creates an entity of type "Solid", like a door
int GameManager::createInteractuableEnemyKillDoorEntity(Vector3f pos, Vector3f size, Vector3f rot, int enemyKills, float respawnTime, bool isRaidZone ,std::vector<SpawnPoint> spawnPointsDoor)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    std::string doorObj { MODEL_OBJECT_DOOR };
    CollisionArea collData { CollisionShape::Box, {2.7f, 2.8f, 0.9f}, 0 };
    GraphicNode* node = graphicsEngine.createAnimatedNode(
        pos,
        size,
        doorObj,
        0.025f // Time between frames
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE | MASK MASK_MAP
        , true  // Is solid
        , false  // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);
    node->setPhysicOffset({0.1f, 2.85f, 0.25f});
    node->playAnimation("closed-idle", false);

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_KILLDOOR);
    entityManager.createComponent<NodeComponent>(entID, node);
    HealthComponent& healthCmp { entityManager.createComponent<HealthComponent>(entID, 1) };
    if(node->hasAnimation())
        healthCmp.totalDeathCounter = 1.0f;
    else
        healthCmp.totalDeathCounter = 0.0f;
        
    InteractableComponent& intCmp { entityManager.createComponent<InteractableComponent>(entID, interactionOpenEnemyKillsDoor, -1,-1,-1, enemyKills) };
    intCmp.spawnPointsInteract = spawnPointsDoor;
    intCmp.interactionData2 = respawnTime;
    intCmp.isRaidZone = isRaidZone;

    // Send online
    if(networkClient.isHost())
    networkClient.send(
        "DoorSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+rot.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+std::to_string(D_KillsDoor)
        +SEP+std::to_string(enemyKills)
    );

    // Return the ID
    return entID;
}

/*Create Interactable Platform Entity*/
// Creates an entity wich move if youi interact
int GameManager::createInteractuablePlatform(Vector3f pos, Vector3f size, bool isRemote, int remoteId, Vector3f posf, AIBehaviourFunction func_to_do)
{
    if(!func_to_do) func_to_do = aiMovingPlatform;

    EntityID entID = entityManager.createEntity();
    //Check if is remote
    int mask=0;
    if(isRemote)
        mask= MASK MASK_MAP;
    else
        mask= MASK MASK_INTERACTABLE | MASK MASK_MAP; 
    // Create its node
    CollisionArea collData { CollisionShape::Box, size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        Vector3f(0.04, 0.04, 0.04)  // Color
    );
    graphicsEngine.createNodeCollisions(
        node
        , mask
        , true  // Is solid
        , false   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    // Create its components+
    entityManager.createComponent<TypeComponent>(entID, ENT_PLATFORM);
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<InteractableComponent>(entID, interactionPlatformControl,-1,-1,remoteId);
    entityManager.createComponent<AIComponent>(entID,USELESS, func_to_do, pos, posf);
    CollisionComponent& collisonCmp = entityManager.createComponent<CollisionComponent>(
        entID,               // ID
        size,     // Size
        Vector3f(0,0,0),          // Gravity
        Vector3f(6,2,6), // Base Speed
        Vector3f(0,0,0)         //OffSet
                            //Falling Speed 
    );
    collisonCmp.moves = true;

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(isRemote ? ME_MovingPlatformRemote : ME_MovingPlatform)
        +SEP SEP SEP+std::to_string(remoteId)
    );

    // Return the ID
    return entID;
}

/*Create Interactable Vending Machine*/
// Creates a vending machien
int GameManager::createVendingMachine(Vector3f pos, Vector3f scale, Vector3f rot ,int type, int price)
{
    EntityID entID = entityManager.createEntity();
    // Create its node
    std::string path="";
    if(type==1)
        path = MODEL_OBJECT_HEAL_VENDING_MACHINE;
    else
        path = MODEL_OBJECT_AMMO_VENDING_MACHINE;
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        scale,
        path
    );
    Vector3f collSize { 1.672, 2.541, 1.199 };
    CollisionArea collData { CollisionShape::Box, collSize * scale, 0 };
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE | MASK MASK_MAP
        , true  // Is solid
        , false   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );
    node->setRotation(rot);
    node->setPhysicRotation(rot);
    node->setPhysicOffset({0.1,2.65,-0.2});

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, type==1 ? ENT_HEAL_VENDINGMACHINE : ENT_AMMO_VENDINGMACHINE);
    entityManager.createComponent<NodeComponent>(entID, node);
    //Una interaccion necesita: ID del pad y su precio
    InteractableComponent& intCmp = entityManager.createComponent<InteractableComponent>(entID, interactionVendingMachine);
    intCmp.interactionData = price;

    // Create and vinculate pad
    float basePadSeparation  { -5.1f };
    Vector3f horizontalAngles   { rot.getHorizontalAngle() };
    float degToRad { 3.1415 / 180 };
    Vector3f padPosition        { 
        cos(rot.y * degToRad) * basePadSeparation
        ,0
        ,-sin(rot.y * degToRad) * basePadSeparation
    };
    std::string padModel = type==1 ? MODEL_OBJECT_HEAL_PAD : MODEL_OBJECT_AMMO_PAD;
    int idPad = createPadEntity(pos+padPosition, {1,1,1}, type, padModel, entID, false);
    auto* padNode { entityManager.getComponentByID<NodeComponent>(idPad)->node };
    padNode->setRotation(rot);
    intCmp.idRemoteToInteractWith = idPad;

    AIComponent* aiCmp = entityManager.getComponentByID<AIComponent>(idPad);
    aiCmp->hasKit=2;

    if(networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+scale.toNetworkString()
        +SEP+rot.toNetworkString()
        +SEP+std::to_string(type==1 ? ME_MedkitVendingMachine : ME_AmmoVendingMachine)
        +SEP SEP SEP SEP+std::to_string(price)
    );

    // Return the ID
    return entID;
}

/*Create Pad Entity*/
// Create a spawner pad of the specified type (1 for health, 2 for ammo)
int GameManager::createPadEntity(Vector3f pos, Vector3f size, int type, std::string path, int machineParentId, bool sendOnline)
{
    EntityID entID = entityManager.createEntity();

    // DIFF
    InteractionFunction interactionFunction;
    AIBehaviourFunction aIBehaviourFunction;
    MapElementType mapElementType;
    if(type == 1)
    {
        interactionFunction = interactionHealPad;
        aIBehaviourFunction = aiHealingPad;
        mapElementType = ME_MedkitBase;
        if(path=="") path = MODEL_OBJECT_HEAL_PAD;
    }
    else
    {
        interactionFunction = interactionAmmoPad;
        aIBehaviourFunction = aiAmmoPad;
        mapElementType = ME_AmmoBase;
        if(path=="") path = MODEL_OBJECT_AMMO_PAD;
    }

    // Create its node
    Vector3f collSize { 1.6, 0.55, 1.6 };
    CollisionArea collData { CollisionShape::Box, collSize * size, 0 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        path
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_MAP
        , true  // Is solid
        , false   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_OTHER);
    entityManager.createComponent<NodeComponent>(entID, node);

    // Create interactable component and save machine parent id (if it has, otherwise is -1)
    auto& intCmp = entityManager.createComponent<InteractableComponent>(entID, interactionFunction);
    intCmp.idRemoteToBeInteracted = machineParentId;

    AIComponent& healPadAI=entityManager.createComponent<AIComponent>(entID, USELESS, aIBehaviourFunction, pos);
    healPadAI.timeWithoutKit = 0;//healPadAI.maxtimeWithoutKit;

    if(sendOnline && networkClient.isHost())
    networkClient.send(
        "MapElementSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+Vector3f().toNetworkString()
        +SEP+std::to_string(mapElementType)
    );

    // Return the ID
    return entID;
}

/*Create Kit Entity*/
// Creates a pickable kit given its
int GameManager::createKitEntity(Vector3f pos, Vector3f size, int type, int parentId, bool parentIsVendingMachine)
{
    EntityID entID = entityManager.createEntity();

    // Get specific data for different kits
    bool isHealth = type == 1;
    std::string kitObj = isHealth ? MODEL_OBJECT_HEAL_KIT : MODEL_OBJECT_AMMO_KIT;
    InteractionFunction interFunc = isHealth ? interactionHealKit : interactionAmmoKit;
    int interactionData = isHealth ? 10 : 20; // Health recovered or ammo percentage recovered

    // If parent is vending machine, obtain pad id from machine
    int padID = parentId;
    if(parentIsVendingMachine)
    {
        auto* machineInter = entityManager.getComponentByID<InteractableComponent>(parentId);
        if(machineInter) padID = machineInter->idRemoteToInteractWith;
    }

    // Create its node
    CollisionArea collData { CollisionShape::Sphere, {0,0,0}, 2 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        kitObj
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE
        , false  // Is solid
        , true   // Triggers an area effect when touched
        , false   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<TypeComponent>(entID, isHealth ? ENT_HEALKIT : ENT_AMMOKIT);
    auto& intCmp = entityManager.createComponent<InteractableComponent>(entID, interFunc, -1,padID,-1, interactionData);
    BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behPickup);
    behCmp.initialPos = pos;
    behCmp.finalPos = pos+Vector3f {0,1,0};
    behCmp.lookAt = Vector3f{0,0,0};

    std::string particlePath = isHealth ? "assets/particles/healparticle.png" : "assets/particles/ammoparticle.png";
    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addKitPickupParticleSystem(particleComponent, particlePath);
    particleComponent.priorityLevel = 2;

    if(networkClient.isHost())
    {
        TypeComponent* type = entityManager.getComponentByID<TypeComponent>(parentId);
        int parentServerId = -1;
        if(type) parentServerId = type->idInServer;
        networkClient.send(
            "PickupSpawned" SEP
            +std::to_string(entID)
            +SEP+pos.toNetworkString()
            +SEP+size.toNetworkString()
            +SEP+std::to_string(isHealth? PK_Medkit : PK_Ammo)
            +SEP+std::to_string(intCmp.interactionData)
            +SEP+std::to_string(parentServerId)
            +SEP+std::to_string(parentIsVendingMachine)
        );
    }

    // Return the ID
    return entID;
}

/*Create Interactable Card Entity*/
// Creates an entity wich you can pick up to your card inventory
int GameManager::createInteractuableCardEntity(Vector3f pos, Vector3f size, int cardID)
{
    EntityID entID = entityManager.createEntity();

    // Create its node
    std::string cardObj= MODEL_OBJECT_CARD;
    CollisionArea collData { CollisionShape::Sphere, {0,0,0}, 2 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        cardObj
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE
        , false  // Is solid
        , false   // Triggers an area effect when touched
        , true   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_PICKUPKEY);
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<InteractableComponent>(entID, interactionReceiveCard, cardID);
    BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behPickup);
    behCmp.initialPos = pos;
    behCmp.finalPos = pos+Vector3f {0,1,0};
    behCmp.lookAt = Vector3f{0,0,0};

    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addKeyParticleSystem(particleComponent);
    particleComponent.priorityLevel = 2;

    if(networkClient.isHost())
    networkClient.send(
        "PickupSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+std::to_string(PK_Key)
        +SEP+std::to_string(cardID)
    );

    // Return the ID
    return entID;
}

/*Create Interactable Weapon*/
// Creates an entity wich you can pick up to add a weapon to your Inventory
int GameManager::createInteractuableWeaponEntity(Vector3f pos, Vector3f size, int weaponID)
{
    EntityID entID = entityManager.createEntity();
    std::string weaponObj;

    switch ((WeaponID) weaponID)
    {
    case W_HANDGUN:
        weaponObj = MODEL_WEAPON_HANDGUN;
        break;
    case W_SHOTGUN:
        weaponObj = MODEL_WEAPON_SHOTGUN;
        break;
    case W_MACHINEGUN:
        weaponObj = MODEL_WEAPON_MACHINEGUN;
        break;
    case W_SNIPER:
        weaponObj = MODEL_WEAPON_SNIPER;
        break;
    case W_CARBINE:
        weaponObj = MODEL_WEAPON_SEMIAUTOMATIC;
        break;
    case W_SPECIAL:
        weaponObj = MODEL_WEAPON_CANNON;
        break;
    case W_EXPLOSIVE:
        weaponObj = MODEL_WEAPON_EXPLOSIVE;
        break;
    default:    //enemy weapon :)
    /* code */
        break;
    }

    // Create its node
    CollisionArea collData { CollisionShape::Sphere, {0,0,0}, 2 };
    GraphicNode* node = graphicsEngine.createNode(
        pos,
        size,
        weaponObj 
    );
    graphicsEngine.createNodeCollisions(
        node
        , MASK MASK_INTERACTABLE
        , false  // Is solid
        , false   // Triggers an area effect when touched
        , true   // Triggers an event when touched
        , collData
    );

    // Create its components
    entityManager.createComponent<TypeComponent>(entID, ENT_PICKUPWEAPON);
    entityManager.createComponent<NodeComponent>(entID, node);
    entityManager.createComponent<InteractableComponent>(entID, interactionReceiveWeapon, weaponID);
    BehaviourComponent& behCmp = entityManager.createComponent<BehaviourComponent>(entID, behPickup);
    behCmp.initialPos = pos;
    behCmp.finalPos = pos+Vector3f {0,1,0};
    behCmp.lookAt = Vector3f{0,0,0};

    //Particles
    auto& particleComponent { entityManager.createComponent<ParticleComponent>(entID) };
    addWeaponPickupParticleSystem(particleComponent);
    particleComponent.priorityLevel = 2;

    if(networkClient.isHost())
    networkClient.send(
        "PickupSpawned" SEP
        +std::to_string(entID)
        +SEP+pos.toNetworkString()
        +SEP+size.toNetworkString()
        +SEP+std::to_string(PK_Weapon)
        +SEP+std::to_string(weaponID)
    );
    
    // Return the ID
    return entID;
}

/*Create Light*/
// Creates a light node so the engine can use it to light the scene
void GameManager::createLight(
    Vector3f pos,
    int type,
    float radius,
    Vector3f intensityAmbient,
    Vector3f intensityDiffuse,
    Vector3f intensitySpecular,
    float attenuationConstant,
    float attenuationLinear,
    float attenuationQuadratic,
    Vector3f direction,
    float innerCutoff,
    float outerCutoff)
{
    if(m_lightList.empty())
        createBaseLight();
    
    int id = graphicsEngine.createLightNode(
        pos
        , type
        , radius
        , intensityAmbient
        , intensityDiffuse
        , intensitySpecular
        , attenuationConstant
        , attenuationLinear
        , attenuationQuadratic
        , direction
        , innerCutoff
        , outerCutoff
    );
    m_lightList.emplace_back(id, radius, pos);
}

/*Create Base Light*/
// Creates the base light for non-dynamic lightning
void GameManager::createBaseLight()
{
    float radius { -1 };
    Vector3f pos {0, 100, 0};
    int id = graphicsEngine.createLightNode(
        pos
        ,1 // Point light
        ,radius
        ,{0.3, 0.3, 0.3} // Max ambient light
        ,{0.9, 0.9, 0.9} // Diffuse
        ,{0.5, 0.5, 0.5} // Specular
        ,1 //light[LIGHT_ATTEN_CONST].GetFloat()
        ,0 //light[LIGHT_ATTEN_LINEAR].GetFloat()
        ,0 //light[LIGHT_ATTEN_QUADRA].GetFloat()
    );
    m_lightList.emplace_back(id, radius, pos);

    int dynid = graphicsEngine.createLightNode(
        pos
        ,1 // Point light
        ,radius
        ,{0.2, 0.2, 0.2} // Max ambient light
        ,{0.7, 0.7, 0.7} // Diffuse
        ,{0.3, 0.3, 0.3} // Specularr
        ,1 //light[LIGHT_ATTEN_CONST].GetFloat()
        ,0.00009 //light[LIGHT_ATTEN_LINEAR].GetFloat()
        ,0.00009 //light[LIGHT_ATTEN_QUADRA].GetFloat()
    );
    m_lightList.emplace_back(dynid, radius, pos);
}

void GameManager::addFireParticleSystem(ParticleComponent& particleComponent)
{
    hyen::ParticleGenerator::CInfo cInfo;
    cInfo.maxParticles =        {80};

    cInfo.texturePath =         {"assets/particles/fire_part2.png"};
    cInfo.origin =              {0,0,0};
    cInfo.gravity =             {0.0f, -2.0f, 0.0f};
    cInfo.mainDir =             {0.0f,  5.0f, 0.0f};
    cInfo.particlesPerSecond =  {80.0f};
    cInfo.spreadFactor =        {-2.5f};
    cInfo.lifeSpan =            {0.5f};
    cInfo.minParticleSize =     {0.5f};
    cInfo.maxParticleSize =     {0.8f};
    cInfo.shapeRadius =         {2.0f};

    cInfo.funcColor =           {&hyen::PGF::generateParticleTransparencyRandomly};

    cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};

    cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionStandard};

    cInfo.funcPos =             {&hyen::PGF::generateRandomPositionBoxShape};

    cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};

    particleComponent.generator = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };
    particleComponent.generator->setActive(false);

    cInfo.maxParticles =        {100};
    cInfo.texturePath =         {"assets/particles/flare2.png"};
    cInfo.origin =              {0,0,0};
    cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    cInfo.mainDir =             {0.0f, 0.0f, 0.0f};
    cInfo.particlesPerSecond =  {100.0f};
    cInfo.spreadFactor =        {0.0f};
    cInfo.lifeSpan =            {0.2f};
    cInfo.minParticleSize =     {0.5f};
    cInfo.maxParticleSize =     {0.75f};
    cInfo.shapeRadius =         {2.0f};

    cInfo.funcColor =           {&hyen::PGF::generateParticleTransparencyRandomly};
    cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};
    cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionStandard};
    cInfo.funcPos =             {&hyen::PGF::generatePositionStatic};
    cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};

    particleComponent.generatorHitmark = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };
    particleComponent.generatorHitmark->setActive(false);
}

void GameManager::addWeaponPickupParticleSystem(ParticleComponent& particleComponent)
{
    hyen::ParticleGenerator::CInfo cInfo;
    cInfo.maxParticles =        {80};
    cInfo.texturePath =         {"assets/particles/weaponparticle.png"};
    cInfo.origin =              {0,0,0};
    cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    cInfo.mainDir =             {0.0f, 0.0f, 0.0f};
    cInfo.particlesPerSecond =  {60.0f};
    cInfo.spreadFactor =        {-0.1f};
    cInfo.lifeSpan =            {1.0f};
    cInfo.minParticleSize =     {0.3f};
    cInfo.maxParticleSize =     {0.5f};
    cInfo.shapeRadius =         {2.0f};

    cInfo.funcColor =           {&hyen::PGF::generateParticleTransparencyRandomly};

    cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};

    cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionHeavyInfluence};

    cInfo.funcPos =             {&hyen::PGF::generateRandomPositionBoxShape};

    cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};

    particleComponent.generator = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };
    particleComponent.generator->setActive(true);
}

void GameManager::addKitPickupParticleSystem(ParticleComponent& particleComponent, std::string path)
{
    hyen::ParticleGenerator::CInfo cInfo;
    cInfo.maxParticles =        {80};

    cInfo.texturePath =         {path};
    cInfo.origin =              {0,0,0};
    cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    cInfo.mainDir =             {0.0f, 0.0f, 0.0f};
    cInfo.particlesPerSecond =  {60.0f};
    cInfo.spreadFactor =        {-0.1f};
    cInfo.lifeSpan =            {0.7f};
    cInfo.minParticleSize =     {0.2f};
    cInfo.maxParticleSize =     {0.5f};
    cInfo.shapeRadius =         {2.0f};

    cInfo.funcColor =           {&hyen::PGF::generateParticleTransparencyRandomly};

    cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};

    cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionHeavyInfluence};

    cInfo.funcPos =             {&hyen::PGF::generateRandomPositionBoxShape};

    cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};

    particleComponent.generator = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };
    particleComponent.generator->setActive(true);
}

void GameManager::addExplosionParticleSystem(ParticleComponent& particleComponent, Vector3f size)
{
    hyen::ParticleGenerator::CInfo cInfo;
    cInfo.maxParticles =        {100};

    cInfo.texturePath =         {"assets/particles/explosionparticle.png"};
    cInfo.origin =              {0,0,0};
    cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    cInfo.mainDir =             {0.0f, 0.0f, 0.0f};
    cInfo.particlesPerSecond =  {100.0f};
    cInfo.spreadFactor =        {-0.8f};
    cInfo.lifeSpan =            {0.8f};
    cInfo.minParticleSize =     {2.0f};
    cInfo.maxParticleSize =     {3.0f};
    cInfo.shapeRadius =         {size.x};

    cInfo.funcColor =           {&hyen::PGF::generateParticleTransparencyRandomly};

    cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};

    cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionHeavyInfluence};

    cInfo.funcPos =             {&hyen::PGF::generateRandomPositionBoxShape};

    cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};

    particleComponent.generator = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };
    particleComponent.generator->setActive(true);
}

void GameManager::addKeyParticleSystem(ParticleComponent& particleComponent)
{
    hyen::ParticleGenerator::CInfo cInfo;
    cInfo.maxParticles =        {80};

    cInfo.texturePath =         {"assets/particles/llavecitasparticle.png"};
    cInfo.origin =              {0,0,0};
    cInfo.gravity =             {0.0f, 0.0f, 0.0f};
    cInfo.mainDir =             {0.0f, 0.0f, 0.0f};
    cInfo.particlesPerSecond =  {60.0f};
    cInfo.spreadFactor =        {-0.1f};
    cInfo.lifeSpan =            {0.7f};
    cInfo.minParticleSize =     {0.2f};
    cInfo.maxParticleSize =     {0.5f};
    cInfo.shapeRadius =         {2.0f};

    cInfo.funcColor =           {&hyen::PGF::generateParticleTransparencyRandomly};

    cInfo.funcSize =            {&hyen::PGF::generateParticleSizeBetween};

    cInfo.funcRandomdir =       {&hyen::PGF::generateRandomDirectionHeavyInfluence};

    cInfo.funcPos =             {&hyen::PGF::generateRandomPositionBoxShape};

    cInfo.funcMaindir =         {&hyen::PGF::generateMainDirectionStandard};

    particleComponent.generator = { static_cast<hyen::ParticleGenerator*>(this->graphicsEngine.createParticleGenerator(&cInfo)) };
    particleComponent.generator->setActive(true);
}

void GameManager::destroyEntity(EntityID entID)
{
    // Destroy engine properties
    NodeComponent* node = entityManager.getComponentByID<NodeComponent>(entID);
    if(node)
    {
        graphicsEngine.unregisterFromAnimationList(node->node);
        graphicsEngine.disableAllCollisionsOf(node->node);
        node->node->destroyInnerNode();

        delete node->node;
        node->node = nullptr;
    }

    // Delete behaviour tree
    auto* aiCmp = entityManager.getComponentByID<AIComponent>(entID);
    if(aiCmp)
        if(aiCmp->behaviourTree) delete aiCmp->behaviourTree;

    ParticleComponent* particle = entityManager.getComponentByID<ParticleComponent>(entID);
    if(particle) 
    {
        if(particle->generator) delete particle->generator;
        if(particle->generatorHitmark) delete particle->generatorHitmark;
        particle->generator = nullptr;
        particle->generatorHitmark = nullptr;
    }


    // Unregister from counters
    TypeComponent* type = entityManager.getComponentByID<TypeComponent>(entID);
    if(type)
    {
        if(type->type == ENT_PLAYER)
        {
            // TODO:: Hide HUD of multiplayer player
            std::vector<int>::iterator it = std::find(m_playerIDs.begin(), m_playerIDs.end(), entID);
            if(it != m_playerIDs.end())
                m_playerIDs.erase(it);
        }
        if(type->type == ENT_ENEMY)
        {
            enemiesInGame--;
            timeWithoutRaid += ENEMY_DEAD;
        }
    }
}

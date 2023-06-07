#include "levelManager.hpp"
#include <rapidjson/filereadstream.h>
#include "levelDataAliases.hpp"
#include "../system/aiBehaviours.hpp"
#include "../system/interactionBehaviours.hpp"
#include "../pathfinding/graph.hpp"
#include "../util/macros.hpp"
#include "../util/jsonReader.hpp"

/*Constructor*/
LevelManager::LevelManager(
    EntityManager& entityMan,
    GameManager& gameMan,
    GraphicsEngine& graphicsEng,
    AudioManager& audio,
    NetworkClient& ntwrk,
    SettingsManager& setman
)
    : entityManager(entityMan), 
    gameManager(gameMan),
    graphicsEngine(graphicsEng),
    audioEngine(audio),
    networkClient(ntwrk),
    settingsManager(setman)
{}

/*Destructor*/
LevelManager::~LevelManager()
{
    if(navmesh) 
        delete navmesh;

    destroyWaypointsAndPaths();
    destroyRoomSensors();
}

/*Next Level*/
// Increases the current level index by 1, and resets to zero if reached the end
// Then loads that level
void LevelManager::nextLevel() noexcept
{
    // Add 1 to current level
    // Return to zero if we reached the last level
    getNextLevelIndex();
    loadLevel(currentLevel);
}

/*Reload Level*/
// Reloads the current level
void LevelManager::reloadLevel() noexcept
{
    loadLevel(currentLevel);
}

/*Load Level*/
// Loads the level in the N position of the level array
void LevelManager::loadLevel(int n) noexcept
{
    // Get map's data name
    rapidjson::Document levelOrderDoc;
    openJsonFile(mapDataFolder+UTIL_MAP_ORDER_FILE, levelOrderDoc);
    this->maxLevels = (int)levelOrderDoc.Size();

    // Check if level index is valid
    if(n < 0 || n >= maxLevels)
    {
        LOG_ERR("Could not load level with index " << n);
        return;
    }

    this->resetWorldFlags();

    // Save current level
    currentLevel = n;

    // Reset world data
    Entity::resetEntityIDAssigner();
    gameManager.resetGameData();

    //Destroy previous pathfinding graph
    if(navmesh) 
    {
        delete navmesh;
        navmesh = nullptr;
    }
    // Destroy navigation data
    this->destroyWaypointsAndPaths();

    // Tell the server to change the map (if host)
    if(networkClient.isHost())
    {
        networkClient.send("MapLoaded" SEP+std::to_string(currentLevel));
        // Tell the server that the map is loaded and that the guests can enter
        networkClient.send("MapReady");
    }

    // Get map data from json
    std::string mapName = levelOrderDoc[n].GetString();
    rapidjson::Document doc;
    openJsonFile(mapDataFolder+mapName, doc);

    // Get important map data sections
    rapidjson::Value& mapStructure   = doc[MAP_SECTION_STRUCTURE];
    rapidjson::Value& mapGenericData = doc[MAP_SECTION_GENERAL_DATA];
    rapidjson::Value& mapElements    = doc[MAP_SECTION_ELEMENTS];
    
    // Restart music
    std::string musicEvent = mapStructure[MAP_DATA_BGMUSIC].GetString();
    audioEngine.stopAllAudioOfType(SOUNDS_MUSIC);
    audioEngine.playSound(musicEvent, SOUNDS_MUSIC);
    audioEngine.setSoundParameterForType(SOUND_PARAM_DANGERLEVEL, gameManager.dangerLevel, SOUNDS_MUSIC);

    // Add the map
    m_lastMapLoaded = mapFolder + mapStructure[MAP_DATA_MESHFILE].GetString();
    graphicsEngine.createMap(m_lastMapLoaded);

    // Get min Y coordinates
    if(mapStructure.HasMember(MAP_DATA_MINYCOORD))
        m_minYCoord = mapStructure[MAP_DATA_MINYCOORD].GetFloat();
    else
        m_minYCoord = -40.f;

    // Create starting player entity
    auto& playerVec = mapStructure[MAP_DATA_PLAYERSPAWN];
    Vector3f playerSpawnPos = GET_VEC3(playerVec);
    Vector3f playerSpawnLookat {-0.5, 0, 0.5};
    if(mapStructure.HasMember(MAP_DATA_PLAYERLOOKAT))
        playerSpawnLookat = GET_VEC3(mapStructure[MAP_DATA_PLAYERLOOKAT]);
    PlayerLook look = audioEngine.getPlayerLook();
    gameManager.createPlayerEntity(playerSpawnPos, playerSpawnLookat, look);

    // Create pathfinding data (if host)
    if(networkClient.isHost())
    {
        //Create Pathfinding Navmesh
        std::string navmeshFile = mapStructure[MAP_DATA_NAVMESHFILE].GetString();
        navmesh = new Graph(graphicsEngine, mapFolder + navmeshFile, true);    //needs to know the level so it can load the mesh
        navmesh->useSmoothPaths = settingsManager.get<bool>("debug/useSmoothPaths");
        
        // Prepare raid data
        gameManager.maxTimeWithoutRaid = mapGenericData[MAP_DATA_RAID_TIME].GetFloat();
        gameManager.enemiesInARaid = mapGenericData[MAP_DATA_ENEMIESINRAID].GetInt();

        // Load level elements (if host) and send them to server
        // Create level exits
        if(mapElements.HasMember(MAP_DATA_LEVEL_EXIT))
        {
            auto& exits = mapElements[MAP_DATA_LEVEL_EXIT];
            for(unsigned int i=0; i<exits.Size(); i++)
            {
                auto& exit = exits[i];
                gameManager.createExitEntity(
                    Vector3f GET_VEC3(exit[LEVELEXIT_POSITION])
                    ,Vector3f GET_VEC3(exit[LEVELEXIT_SCALE])
                    //,Vector3f GET_VEC3(exit[LEVELEXIT_ROTATION])
                );
            }
        }
        // Create enemy kill Doors
        if(mapElements.HasMember(MAP_DATA_ELEM_KILLDOORS))
        {
            auto& elemKillDoors = mapElements[MAP_DATA_ELEM_KILLDOORS];
            for(unsigned int i=0; i<elemKillDoors.Size(); i++)
            {
                auto& door = elemKillDoors[i];

                // Get Spawn points
                std::vector<SpawnPoint> spawnpts;
                if(door.HasMember(KILLDOOR_SPAWNPOINTS))
                {
                    auto& spawnptJsonVec = door[KILLDOOR_SPAWNPOINTS];
                    for(unsigned int j=0; j<spawnptJsonVec.Size(); j++)
                    {
                        SpawnPoint spawnPt = getSpawnpointFrom(spawnptJsonVec[j]);
                        spawnpts.push_back(spawnPt);
                    }
                }

                gameManager.createInteractuableEnemyKillDoorEntity(
                    Vector3f GET_VEC3(door[KILLDOOR_POSITION]),
                    Vector3f GET_VEC3(door[KILLDOOR_SCALE]),
                    Vector3f GET_VEC3(door[KILLDOOR_ROTATION]),
                    door[KILLDOOR_KILLS_REQUIRED].GetInt(),
                    door[KILLDOOR_RESPAWN_TIME].GetFloat(),
                    door[KILLDOOR_RAID_ZONE].GetBool(),
                    spawnpts
                );
            }
        }
        // Create key-locked Doors
        if(mapElements.HasMember(MAP_DATA_ELEM_KEYDOORS))
        {
            auto& doors = mapElements[MAP_DATA_ELEM_KEYDOORS];
            for(unsigned int i=0; i<doors.Size(); i++)
            {
                auto& door = doors[i];

                // Get Spawn points
                std::vector<SpawnPoint> spawnpts;
                if(door.HasMember(KEYDOOR_SPAWNPOINTS))
                {
                    auto& spawnptJsonVec = door[KEYDOOR_SPAWNPOINTS];
                    for(unsigned int j=0; j<spawnptJsonVec.Size(); j++)
                    {
                        SpawnPoint spawnPt = getSpawnpointFrom(spawnptJsonVec[j]);
                        spawnpts.push_back(spawnPt);
                    }
                }

                gameManager.createInteractuableDoorWithCardEntity(
                    Vector3f GET_VEC3(door[KEYDOOR_POSITION]),
                    Vector3f GET_VEC3(door[KEYDOOR_SCALE]),
                    Vector3f GET_VEC3(door[KEYDOOR_ROTATION]),
                    door[KEYDOOR_KEY_ID].GetInt(),
                    door[KEYDOOR_REMOTE_ID].GetInt(),
                    door[KILLDOOR_RAID_ZONE].GetBool(),
                    door[KEYDOOR_RESPAWN_TIME].GetInt(),
                    spawnpts
                );
            }
        }
        // Create keys
        if(mapElements.HasMember(MAP_DATA_ELEM_KEYS))
        {
            auto& keys = mapElements[MAP_DATA_ELEM_KEYS];
            for(unsigned int i=0; i<keys.Size(); i++)
            {
                auto& key = keys[i];
                gameManager.createInteractuableCardEntity(
                    Vector3f GET_VEC3(key[KEY_POSITION]),
                    Vector3f GET_VEC3(key[KEY_SCALE]),
                    key[KEY_PARAM_ID].GetInt()
                );
            }
        }
        // Create pad
        if(mapElements.HasMember(MAP_DATA_PADS))
        {
            auto& pads = mapElements[MAP_DATA_PADS];
            for(unsigned int i=0; i<pads.Size(); i++)
            {
                auto& pad = pads[i];
                gameManager.createPadEntity(
                    Vector3f GET_VEC3(pad[PAD_POSITION]),
                    Vector3f GET_VEC3(pad[PAD_SCALE]),
                    pad[PAD_TYPE].GetInt()
                );
            }
        }
        // Create vending machines
        if(mapElements.HasMember(MAP_DATA_VENDING_MACH))
        {
            auto& machs = mapElements[MAP_DATA_VENDING_MACH];
            for(unsigned int i=0; i<machs.Size(); i++)
            {
                auto& mach = machs[i];
                gameManager.createVendingMachine(
                    Vector3f GET_VEC3(mach[VENDINGMACH_POSITION]),
                    Vector3f GET_VEC3(mach[VENDINGMACH_SCALE]),
                    Vector3f GET_VEC3(mach[VENDINGMACH_ROTATION]),
                    mach[VENDINGMACH_TYPE].GetInt(),
                    mach[VENDINGMACH_PRICE].GetInt()
                );
            }
        }
        // Create pickup weapon
        if(mapElements.HasMember(MAP_DATA_PICKUP_WEAPON))
        {
            auto& weapons = mapElements[MAP_DATA_PICKUP_WEAPON];
            for(unsigned int i=0; i<weapons.Size(); i++)
            {
                auto& weapon = weapons[i];
                gameManager.createInteractuableWeaponEntity(
                    Vector3f GET_VEC3(weapon[PICKWEAPON_POSITION]),
                    Vector3f GET_VEC3(weapon[PICKWEAPON_SCALE]),
                    weapon[PICKWEAPON_TYPE].GetInt()
                );
            }
        }
        // Create pre-spawned enemies
        if(mapElements.HasMember(MAP_DATA_ENEMIES))
        {
            auto& enemies = mapElements[MAP_DATA_ENEMIES];
            for(unsigned int i=0; i<enemies.Size(); i++)
            {
                auto& enemy = enemies[i];
                gameManager.createEnemyEntity(
                    enemy[PRESPAWNEDENEMY_TYPE].GetInt()
                    ,Vector3f GET_VEC3(enemy[PRESPAWNEDENEMY_POSITION])
                    ,Vector3f GET_VEC3(enemy[PRESPAWNEDENEMY_SCALE])
                    ,Vector3f GET_VEC3(enemy[PRESPAWNEDENEMY_ROTATION])
                    ,enemy[PRESPAWNEDENEMY_PATROL].GetInt()
                    ,enemy[PRESPAWNEDENEMY_INIT_PT].GetInt()
                    ,enemy[PRESPAWNEDENEMY_ISRAID].GetBool()
                );
            }
        }
        if(mapElements.HasMember(MAP_DATA_INVISIBLE_WALLS))
        {
            auto& walls = mapElements[MAP_DATA_INVISIBLE_WALLS];
            for(unsigned int i=0; i<walls.Size(); i++)
            {
                auto& wall = walls[i];
                gameManager.createSolidEntity(
                     Vector3f GET_VEC3(wall[INVISIBLE_WALL_POSITION])
                    ,Vector3f GET_VEC3(wall[INVISIBLE_WALL_SIZE])
                );
            }
        }

        // Create enemy spawn points
        auto& enemySpawnPoints = mapGenericData[MAP_DATA_ENEMYSPAWNPTS];
        gameManager.spawnArray.reserve(enemySpawnPoints.Size());
        for(unsigned int i=0; i<enemySpawnPoints.Size(); i++)
        {
            SpawnPoint spawnPt = getSpawnpointFrom(enemySpawnPoints[i]);
            gameManager.spawnArray.push_back(spawnPt);
        }

        // Create AI waypoints and routes (if host)
        auto& waypointVec = mapStructure[MAP_DATA_WAYPOINTS];
        waypoints.reserve(waypointVec.Size());
        for(unsigned int i=0; i<waypointVec.Size(); i++)
        {
            auto& wayp = waypointVec[i];
            waypoints.emplace_back(Vector3f GET_VEC3(wayp));
        }
        auto& routeVec = mapStructure[MAP_DATA_ROUTES];
        routes.reserve(routeVec.Size());
        for(unsigned int i=0; i<routeVec.Size(); i++)
        {
            auto& route = routeVec[i];
            if(route.Size() > 0)
            {
                std::vector<Waypoint*> tmpRoute;
                tmpRoute.reserve(route.Size());
                for(unsigned int j = 0; j<route.Size(); j++)
                    tmpRoute.emplace_back(&waypoints[ route[j].GetInt() ]);
                routes.emplace_back(tmpRoute);
                tmpRoute.clear();
            }
        }

        // Tell guests to load the map
        networkClient.send("LevelSelected");
    }
    else // Is not host
    {
        // Ask for the world state to the server
        networkClient.send("GetWorldStatus");
    }

    // Create lights
    gameManager.clearLightList();
    if(mapElements.HasMember(MAP_DATA_LIGHTS))
    {
        auto& lights = mapElements[MAP_DATA_LIGHTS];
        for(unsigned int i=0; i<lights.Size(); i++)
        {
            auto& light = lights[i];
            gameManager.createLight(
                Vector3f GET_VEC3(light[LIGHT_POSITION])
                ,light[LIGHT_TYPE].GetInt()
                ,light[LIGHT_RADIUS].GetFloat()
                ,Vector3f GET_VEC3(light[LIGHT_INTENSITY_AMB])
                ,Vector3f GET_VEC3(light[LIGHT_INTENSITY_DIF])
                ,Vector3f GET_VEC3(light[LIGHT_INTENSITY_SPE])
                ,light[LIGHT_ATTEN_CONST].GetFloat()
                ,light[LIGHT_ATTEN_LINEAR].GetFloat()
                ,light[LIGHT_ATTEN_QUADRA].GetFloat()
                ,Vector3f GET_VEC3(light[LIGHT_DIRECTION])
                ,light[LIGHT_INNERCUTOFF].GetFloat()
                ,light[LIGHT_OUTERCUTOFF].GetFloat()
            );
        }
        gameManager.setDynamicLightning( graphicsEngine.getDynamicLightning() );
    }

    destroyRoomSensors();
    if(mapElements.HasMember(MAP_DATA_ROOMSENSORS))
    {
        auto& sensors = mapElements[MAP_DATA_ROOMSENSORS];
        m_roomSensors.reserve(sensors.Size());
        for(unsigned int i=0; i<sensors.Size(); ++i)
        {
            auto& sensor = sensors[i];
            m_roomSensors.emplace_back(
                 Vector3f GET_VEC3(sensor[SENSOR_CORNERFROM])
                ,Vector3f GET_VEC3(sensor[SENSOR_CORNERTO])
            );
        }
    }
}

/*Get Current Level Index*/
// Returns the current level's index
int LevelManager::getCurrentLevelIndex()
{
    return currentLevel;
}

/*Get Next Level Index*/
// Adds 1 to the current level and returns the currentLevel
int LevelManager::getNextLevelIndex()
{
    if(++currentLevel >= maxLevels)
    {
        currentLevel = 0;
    }
    return currentLevel;
}

/*Force Current Level*/
// Sets the current level index
void LevelManager::forceCurrentLevel(int newLevel)
{
    this->currentLevel = newLevel;
}

/*Update Occlusion By Physics*/
// Updates occlusion by physics (hiding nodes that are not in a "room" the player is in)
void LevelManager::updateOcclusionByPhysics()
{
    // Update timer
    m_physicOcclusionTimer -= graphicsEngine.getTimeDiffInSeconds();
    if(m_physicOcclusionTimer >= 0) return;
    m_physicOcclusionTimer = m_physicOcclusionInterval;

    int playerID { gameManager.getLocalPlayerID() };
    auto playerPos { entityManager.getComponentByID<NodeComponent>(playerID)->node->getPosition() };
    std::list<RoomSensor*> playerRooms {};

    // Find rooms the player is in
    for(unsigned int i=0; i<m_roomSensors.size(); ++i)
        if(pointIsInRoom(playerPos, m_roomSensors[i]))
            playerRooms.push_back(&m_roomSensors[i]);
    
    auto& nodes { entityManager.getComponentVector<NodeComponent>() };
    for(auto& node : nodes)
    {
        // Ignore local player
        if((int)node.getEntityID() == playerID) continue;

        auto* innerNode { node.node };
        auto nodePos { innerNode->getPosition() };
        bool inPlayerRoom { false };
        for(auto* room : playerRooms)
        {
            if(pointIsInRoom(nodePos, *room))
            {
                inPlayerRoom = true;
                break;
            }
        }
        innerNode->setVisible(inPlayerRoom);
    }
}

/*Reset World Flags*/
// Resets world flags that mark events such as "level finished" or "reload level"
constexpr void LevelManager::resetWorldFlags() noexcept
{
    flagLevelFinished = false;
    flagGoToNextLevel = false;
    flagGoBackToSelection = false;
    flagReloadLevel = false;
    flagPlayerDied = false;
}

void LevelManager::destroyWaypointsAndPaths() noexcept
{
    routes.clear();
    waypoints.clear();
}

void LevelManager::destroyRoomSensors() noexcept
{
    //for(auto* sensor : m_roomSensors)
    //    delete sensor;
    //if(frustumSensor) delete frustumSensor;
    //frustumSensor = nullptr;
    m_roomSensors.clear();
}

bool LevelManager::pointIsInRoom(Vector3f point, RoomSensor& room)
{
    auto from { room.cornerFrom };
    auto to { room.cornerTo };
    return
        from.x < point.x && point.x < to.x
            &&
        from.y < point.y && point.y < to.y
            &&
        from.z < point.z && point.z < to.z
    ;
}

/*Get Spawn Point From Json Value*/
// Receives a rapidjson element and extracts a spawnpoint from it
// If the value is not eligible to be a SpawnPoint unexpected things can happen
SpawnPoint LevelManager::getSpawnpointFrom(rapidjson::Value& spawnPt)
{
    if(spawnPt.HasMember(SPAWNPOINT_OPT_NUM_ENEMIES))
    {
        return SpawnPoint(
            Vector3f GET_VEC3(spawnPt[SPAWNPOINT_POSITION])
            , spawnPt[SPAWNPOINT_PATROL].GetInt()
            , spawnPt[SPAWNPOINT_INIT_PATROL_PT].GetInt()
            , spawnPt[SPAWNPOINT_ENEMY_TYPE].GetInt()
            , spawnPt[SPAWNPOINT_IS_FOR_RAID].GetBool()
            , spawnPt[SPAWNPOINT_OPT_NUM_ENEMIES].GetInt()
        );
    }
    else
    {
        return SpawnPoint(
            Vector3f GET_VEC3(spawnPt[SPAWNPOINT_POSITION])
            , spawnPt[SPAWNPOINT_PATROL].GetInt()
            , spawnPt[SPAWNPOINT_INIT_PATROL_PT].GetInt()
            , spawnPt[SPAWNPOINT_ENEMY_TYPE].GetInt()
            , spawnPt[SPAWNPOINT_IS_FOR_RAID].GetBool()
        );
    }
}


/*AI FUNCTIONS*/
/*Find Path from*/
//Returns a vector that contains the GraphNodes that make the path from origin to destination
std::list<Vector3f> LevelManager::findPath(Vector3f origin, Vector3f destination)
{
    return navmesh->findPathAStar(origin,destination);
}

/*Is In Walkable mesh*/
// Return if the position, pos, is inside a walkable mesh
bool LevelManager::isInWalkableMesh(Vector3f pos)
{
    return navmesh->isInTriangle(pos);
}

/*Get Route*/
// Gets one of the map routes
std::vector<Waypoint*>* LevelManager::getRoute(int route)
{
    if(route >= 0 && route < (int)routes.size())
        return &routes[route];
    else
        return nullptr;
}

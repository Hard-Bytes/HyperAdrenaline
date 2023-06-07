#pragma once
#include "../ecs/entityManager.hpp"
#include "../manager/audioManager.hpp"
#include "../manager/hudManager.hpp"
#include "../manager/saveDataManager.hpp"
#include "../network/websocketClient.hpp"

class GameManager
{
public:
    GameManager(
        EntityManager& entman
        , GraphicsEngine& graph
        , AudioManager& audio
        , HUDManager& hudMan
        , NetworkClient& netcli
        , SettingsManager& setman
        , SaveDataManager& saveMan
    );
    ~GameManager();

    // Node treatment
    void disableNode( GraphicNode& node);

    // Player IDs
    const std::vector<int>& getPlayerIDs() const noexcept {return m_playerIDs;}
    int getLocalPlayerID() const noexcept;

    // Reseters
    void resetGameData();
    void clearGame();
    void updateDeadEntities();

    // Respawn/Raid Data
    int enemiesInGame {0};
    bool inARaid{0};
    int killedMapEnemies{0};
    // Max respawn/raid data (can change between levels, not const)
    int maxEnemies {10};
    int enemiesInARaid {8};
    bool isRaidZone {false};
    
    // Respawn timers
    float timeWithoutMaxEnemies {0.0f};
    float timeWithoutMaxEnemiesInARaid {0.0f};
    float timeWithoutRaid {0.f};  
    // Max respawn timers (can change between levels, not const)
    float maxtimeWithoutMaxEnemies{8000}; 
    float maxTimeWithoutRaid {50000};

    // Spawn data
    std::vector<SpawnPoint> spawnArray {};

    // Music intensity
    int dangerLevel {1};

    // Game configs
    bool showFPS { true };
    bool showUI  { true };

    // Online data that goes nowhere else
    const std::vector<std::string>& getPlayerNames() const noexcept { return m_playerNames; }
    void addPlayerNameToList(const std::string& name) noexcept;
    void removePlayerNameFromList(const std::string& name) noexcept;
    void setPlayerName(const std::string& name) noexcept { m_playerName = name; }
    std::string getPlayerName() const noexcept { return m_playerName; }
    int m_menuSelection {-1};
    bool m_selectionAccepted { false };

    // Lights
    void updateDynamicLightning(bool force = false) noexcept;
    void setDynamicLightning(bool isDynamic, bool updateLights = true) noexcept;
    void clearLightList() noexcept;

    // Player entities
    int createPlayerEntity(Vector3f pos, Vector3f initLookAt, PlayerLook look=PL_Liam);
    int createNPCPlayerEntity(Vector3f pos={0,0,0}, PlayerLook look=PL_Liam);
    int createCameraEntity(Vector3f pos, Vector3f initLookAt);

    // Enemy entities
    int createEnemyEntity(int type, Vector3f pos, Vector3f size, Vector3f rot, int path=-1, int initWaypointIndex = 0,bool isRaidEnemy = false);
    int createDummyEntity(Vector3f, Vector3f size, Vector3f rot);
    int createMeleeEnemyEntity(Vector3f pos, Vector3f size, Vector3f rot, int path=-1, int initWaypointIndex = 0, bool isRaidEnemy = false);
    int createShootingEnemyEntity(Vector3f pos, Vector3f size, Vector3f rot, int path=-1, int initWaypointIndex = 0, bool isRaidEnemy = false, bool isLongRange=false);
    int createKamikazeEnemyEntity(Vector3f pos, Vector3f size, Vector3f rot, float damage = 0, int path=-1, int initWaypointIndex = 0,bool isRaid = false);

    // Obstacle entities
    int createExitEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10});
    int createSolidEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10});

    // Hazard entities
    int createKillingZoneEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10});
    int createDamagingZoneEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, float damagePerSecond=1.0f);
    int createDamagingZonePerTimeEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10},Vector3f rot={0,0,0}, Vector3f speed={0,0,0}, float damagePerSecond=1.0f, float lifeTime=0.0f, bool collides=true, std::string model="", std::string sound="", EntityType eType = ENT_ATTACKAREA);
    int createExpandableDamagingZonePerTimeEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10},Vector3f rot={0,0,0}, Vector3f speed={0,0,0}, float damagePerSecond=1.0f, float lifeTime=0.0f, std::string model="", std::string sound="",EntityType eType = ENT_EXPLOSIONAREA);
    int createExplosionMisilEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10},Vector3f rot={0,0,0}, Vector3f speed={0,0,0}, float damage=1.0f, std::string model="", std::string bulletSound="",std::string explosionSound="");
    int createEnemyBullet (Vector3f pos, Vector3f size,Vector3f rotate,Vector3f speed, float damage, float time, std::string sound);

    // Interactable: Remotes (Buttons)
    int createInteractuableRemoteEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, int idToInteract=-1);
    int createInteractuableRemoteWithCardEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, int CardID=-1, int idToInteract=-1, Vector3f rotate={0,0,0});
    
    // Interactable: Doors
    int createInteractuableDoorEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, Vector3f rot={0,0,0}, bool isRemote=false, int remoteId=-1, std::vector<SpawnPoint> spawnPointsDoor={});
    int createInteractuableDoorWithCardEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, Vector3f rot={0,0,0}, int CardID=-1, int remoteId=-1, bool isRaidZone=true,float respawnTime = 8000.0f, std::vector<SpawnPoint> spawnPointsDoor={});
    int createInteractuableDoorWithPointsEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, Vector3f rot={0,0,0}, int doorPoints=100, std::vector<SpawnPoint> spawnPointsDoor={});
    int createInteractuableEnemyKillDoorEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, Vector3f rot={0,0,0}, int enemyKills=0, float respawnTime = 8000.0f, bool isRaidZone=true, std::vector<SpawnPoint> spawnPointsDoor={});
   
    // Interactable: Other
    int createInteractuablePlatform(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, bool isRemote=false, int remoteId=-1, Vector3f posf={0,0,0}, AIBehaviourFunction=nullptr);
    
    // Map elements
    int createVendingMachine(Vector3f pos, Vector3f size, Vector3f rot, int type, int price);
    int createPadEntity(Vector3f pos, Vector3f size, int type, std::string path="", int machineParentId = -1, bool sendOnline = true);
    
    //Pickups
    int createKitEntity(Vector3f pos, Vector3f size, int type, int parentId, bool parentIsVendingMachine = false);
    int createInteractuableCardEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, int CardID=-1);
    int createInteractuableWeaponEntity(Vector3f pos={0,0,0}, Vector3f size={10,10,10}, int WeaponID=-1);

    // Lights
    void createLight(
        Vector3f pos,
        int type,
        float radius,
        Vector3f intensityAmbient,
        Vector3f intensityDiffuse,
        Vector3f intensitySpecular,
        float attenuationConstant,
        float attenuationLinear,
        float attenuationQuadratic,
        Vector3f direction = {0,0,0},
        float innerCutoff = 0,
        float outerCutoff = 0
    );
    void createBaseLight();

private:
    EntityManager& entityManager;
    GraphicsEngine& graphicsEngine;
    AudioManager& audioEngine;
    HUDManager& hudManager;
    NetworkClient& networkClient;
    SettingsManager& settingsManager;
    SaveDataManager& saveDataManager;

    struct LightData {
        LightData(int p_id, float p_rad, Vector3f p_pos) :
            position(p_pos), lightID(p_id), radius(p_rad), baseRadius(p_rad)
        {}
        ~LightData() = default;
        Vector3f position;
        int lightID  {-1 };
        float radius { 0 };
        float baseRadius { -1 };
    };
    std::list<LightData> m_lightList;
    float m_dynamicLightUpdateInterval { 0.5f };
    float m_dynamicLightUpdateTimer    { 0.0f };

    std::vector<int> m_playerIDs;
    std::vector<std::string> m_playerNames;
    std::string m_playerName;

    void addFireParticleSystem(ParticleComponent& ptclCmp);
    void addWeaponPickupParticleSystem(ParticleComponent& ptclCmp);
    void addKitPickupParticleSystem(ParticleComponent& ptclCmp, std::string path);
    void addExplosionParticleSystem(ParticleComponent& particleComponent, Vector3f size);
    void addKeyParticleSystem(ParticleComponent& particleComponent);

    void destroyEntity(EntityID entID);
};

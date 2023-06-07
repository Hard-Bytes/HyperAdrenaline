#pragma once
#include <string>
#include "../facade/graphicsEngine.hpp"
#include "../manager/audioManager.hpp"
#include "../network/websocketClient.hpp"
#include "../ecs/entityManager.hpp"
#include "../pathfinding/graph.hpp"


class LevelManager
{
public:
    // Constructor
    explicit LevelManager(
        EntityManager& entityMan,
        GameManager& gameMan,
        GraphicsEngine& graphicsEng,
        AudioManager& audio,
        NetworkClient& ntwrk,
        SettingsManager& setman
    );
    ~LevelManager();

    void nextLevel() noexcept;
    void reloadLevel() noexcept;
    void loadLevel(int n) noexcept;

    int getCurrentLevelIndex();
    int getNextLevelIndex();
    void forceCurrentLevel(int newLevel);

    void updateOcclusionByPhysics();

    float getMinimumYCoord() const noexcept
        { return m_minYCoord; }
    const std::string& getLastMapLoaded()
        { return m_lastMapLoaded; }

    // World interaction flags
    bool flagLevelFinished {false};
    bool flagGoToNextLevel {false};
    bool flagGoBackToSelection {false};
    bool flagReloadLevel {false};
    bool flagPlayerDied {false};

    constexpr void resetWorldFlags() noexcept;

    // AI Functions
    std::list<Vector3f> findPath(Vector3f origin, Vector3f destination);
    bool isInWalkableMesh(Vector3f pos);

    // AI Waypointing
    std::vector<Waypoint> waypoints;
    std::vector<std::vector<Waypoint*>> routes;
    std::vector<Waypoint*>* getRoute(int route);
private:
    const std::string mapFolder{UTIL_MAP_MODEL_FOLDER};
    const std::string mapDataFolder{UTIL_MAP_DATA_FOLDER};

    std::string m_lastMapLoaded {};

    int maxLevels{0};
    int currentLevel { 0 };

    float m_minYCoord { -20.0f };

    struct RoomSensor {
        explicit RoomSensor(Vector3f from, Vector3f to)
            : cornerFrom(from), cornerTo(to)
        {
            if(cornerFrom.x > cornerTo.x) {
                float tmp { cornerFrom.x };
                cornerFrom.x = cornerTo.x;
                cornerTo.x = tmp;
            }
            if(cornerFrom.y > cornerTo.y) {
                float tmp { cornerFrom.y };
                cornerFrom.y = cornerTo.y;
                cornerTo.y = tmp;
            }
            if(cornerFrom.z > cornerTo.z) {
                float tmp { cornerFrom.z };
                cornerFrom.z = cornerTo.z;
                cornerTo.z = tmp;
            }
        }
        ~RoomSensor() = default;
        Vector3f cornerFrom;
        Vector3f cornerTo;
    };
    std::vector<RoomSensor> m_roomSensors {};
    float m_physicOcclusionTimer { 0.0f };
    float m_physicOcclusionInterval { 0.5f }; // In seconds

    EntityManager& entityManager;
    GameManager& gameManager;
    GraphicsEngine& graphicsEngine;
    AudioManager& audioEngine;
    NetworkClient& networkClient;
    SettingsManager& settingsManager;

    Graph* navmesh{nullptr};

    // Auxiliar
    void destroyWaypointsAndPaths() noexcept;
    void destroyRoomSensors() noexcept;
    bool pointIsInRoom(Vector3f point, RoomSensor& room);
    SpawnPoint getSpawnpointFrom(rapidjson::Value&);
};

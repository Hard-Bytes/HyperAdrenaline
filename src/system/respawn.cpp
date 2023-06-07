#include "respawn.hpp"

// Constructor
RespawnSystem::RespawnSystem(GameContext& gc)
    : gctx(gc)
{}

// Destructor
RespawnSystem::~RespawnSystem()
{

}

/*Respawn Enemies*/
void RespawnSystem::respawnEnemies() const noexcept
{
    GameManager& gameManager = gctx.gameManager;
    if(gameManager.enemiesInGame < gameManager.maxEnemies)
    {
        gameManager.timeWithoutMaxEnemies += gctx.graphicsEngine.getTimeDiff();
        if(gameManager.enemiesInGame<=1)
            gameManager.timeWithoutMaxEnemies += gctx.graphicsEngine.getTimeDiff();
        if(gameManager.timeWithoutMaxEnemies >= gameManager.maxtimeWithoutMaxEnemies)
        {
            gameManager.timeWithoutMaxEnemies=0;

            std::vector<int> playerIDs = gameManager.getPlayerIDs();
            NodeComponent* nodePlayer = gctx.entityManager.getComponentByID<NodeComponent>(playerIDs[0]);
            Vector3f playerPos = nodePlayer->node->getPosition();
            for(SpawnPoint& spawnData : gctx.gameManager.spawnArray)
            {
                int distance = (playerPos-spawnData.position).length();
                if(spawnData.numEnemiesToSpawn > 0 && distance >= MIN_DISTANCE && distance <= MAX_DISTANCE)
                {
                    gameManager.createEnemyEntity
                    (
                        spawnData.enemyType,
                        spawnData.position,
                        Vector3f{1,1,1},
                        Vector3f{0,0,0},
                        spawnData.patrolPath,
                        spawnData.initWaypointIndex
                    );
                    --spawnData.numEnemiesToSpawn;
                }
            }
        }          
    }
}

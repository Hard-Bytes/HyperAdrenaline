#include "raid.hpp"
#include "../util/macros.hpp"
#include "../util/paths.hpp"

// Constructor
RaidSystem::RaidSystem(GameContext& gc)
    : gctx(gc)
{}

// Destructor
RaidSystem::~RaidSystem()
{

}

/*Respawn Enemies*/
void RaidSystem::respawnEnemies()
{
    GameManager& gameManager = gctx.gameManager;
    if(gameManager.inARaid)
    {
        gameManager.timeWithoutMaxEnemiesInARaid += gctx.graphicsEngine.getTimeDiff();
        if(gameManager.enemiesInGame!=0)
            gameManager.timeWithoutMaxEnemiesInARaid -= gctx.graphicsEngine.getTimeDiff()/2;

        if(gameManager.timeWithoutMaxEnemiesInARaid >= gameManager.maxtimeWithoutMaxEnemies)
        {
            gameManager.timeWithoutMaxEnemiesInARaid = 0;

            std::vector<int> playerIDs = gameManager.getPlayerIDs();
            NodeComponent* nodePlayer = gctx.entityManager.getComponentByID<NodeComponent>(playerIDs[0]);
            Vector3f playerPos = nodePlayer->node->getPosition();
            int enemiesInARound=0;
            for(SpawnPoint& spawnData : gctx.gameManager.spawnArray)
            {
                if(enemiesSpawned < gameManager.enemiesInARaid){
                    int distance = (playerPos-spawnData.position).length();
                    if(enemiesInARound < gameManager.enemiesInARaid/2  &&
                        gameManager.enemiesInGame < gameManager.maxEnemies &&
                        distance >= MIN_DISTANCE_RAID &&
                        distance <= MAX_DISTANCE_RAID
                    )
                    {
                        
                        enemiesInARound++;
                        enemiesSpawned++;
                        gameManager.createEnemyEntity
                        (
                            spawnData.enemyType,
                            spawnData.position,
                            Vector3f{1,1,1},
                            Vector3f{0,0,0},
                            spawnData.patrolPath,
                            spawnData.initWaypointIndex,
                            true
                        );
                    }
                }else
                {
                    //Terminar raid
                    enemiesSpawned=0;
                    gameManager.inARaid=false;
                    gameManager.timeWithoutRaid=0;
                }
            }
        }          
    }
    else
    {
        if(gameManager.isRaidZone)
        {
            if(gameManager.timeWithoutRaid<0)
            {
                gameManager.timeWithoutRaid=0;
            }
            float dt=gctx.graphicsEngine.getTimeDiff();
            gameManager.timeWithoutRaid+=dt;
            if(gameManager.timeWithoutRaid >=  gameManager.maxTimeWithoutRaid)
            {
                gameManager.inARaid=true;
                gctx.audioManager.playSound(SOUND_VFX_RAID_START, SOUNDS_SFX_AMBIENCE);
                gameManager.timeWithoutMaxEnemiesInARaid = gameManager.maxtimeWithoutMaxEnemies;

            }
        }

    }
}

#include "behaviourNodeActions.hpp"
#include <limits>
#include "../components/aiCmp.hpp"
#include "../context/gameContext.hpp"

int getPlayerID(GameManager& gameManager,int targetPlayerID)
{
    std::vector<int> players = gameManager.getPlayerIDs();
    
    for(unsigned int i = 0; i<players.size();++i)
    {
        if(players[i] == targetPlayerID)
            return players[i];
    }
    return players[0];
}

Vector3f getPlayerPos(GameContext& gctx, int targetPlayerID)
{
    // Get player
    int id = getPlayerID(gctx.gameManager, targetPlayerID);
    NodeComponent* playerNode = gctx.entityManager.getComponentByID<NodeComponent>(id);
    return playerNode->node->getPosition();
}

/*Behaviour: Selector*/
// Checks every child until one succeeds or everyone fails
ResultType behSelector(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    for(unsigned int child = 0; child < treeNode.childs.size(); child++)
    {
        ResultType res = treeNode.childs[child]->ping(cmp, gctx);
        if(res == BT_SUCCESS || res == BT_RUNNING)
            return res;
    }
    return BT_FAILURE;
}

/*Behaviour: Sequence*/
// Checks every child until one fails or everyone succeeds
ResultType behSequence(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    for(unsigned int child = 0; child < treeNode.childs.size(); child++)
    {
        ResultType res = treeNode.childs[child]->ping(cmp, gctx);
        if(res == BT_FAILURE || res == BT_RUNNING)
            return res;
    }
    return BT_SUCCESS;
}

/*Behaviour: Succeder*/
// Returns Success, if child returns failure or success, but not if the child returns running
ResultType behSucceder(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    ResultType response = BT_SUCCESS ; 

    if(treeNode.childs.size()>0)
    {
        response = treeNode.childs[0]->ping(cmp, gctx);

        if(response == BT_FAILURE)
            response = BT_SUCCESS;

    }

    return response;
}

/*Behaviour: Inverter*/
// Returns the negated version of the input (success or failure) but not if the child returns running
ResultType behInverter(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    ResultType response = BT_SUCCESS ; 

    if(treeNode.childs.size()>0)
    {
        response = treeNode.childs[0]->ping(cmp, gctx);

        if(response == BT_FAILURE)
            response = BT_SUCCESS;
        else if(response == BT_SUCCESS)
            response = BT_FAILURE;

    }

    return response;
}

/*Behaviour: DEBUG Inverter*/
// Always returns failure, regardless of child respondes (does not ping childs)
ResultType behDebugNegator(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    return BT_FAILURE;
}

/*Behaviour (Leaf): Walk*/
// Calculates route to the position defined in cmp.destination
// Returns running if still on its way, success if it reached the destination, or failure if it can't reach
ResultType behWalk(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    if(cmp.currentPath.size() == 0)
    {
        // Calculate path to destination
        Vector3f pos = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID())->node->getPosition();
        cmp.currentPath = gctx.levelManager.findPath(pos, cmp.destination);

        // If path is still empty, we can't reach our destination
        if(cmp.currentPath.size() == 0)
            return BT_FAILURE;
    }

    // Return success if we reached the destination
    if(cmp.currentPath.size() == 0)
        return BT_SUCCESS;
    // Return running if we're still on our way
    else
        return BT_RUNNING;     
}

/*Behaviour (Leaf): See Player (Condition)*/
// Throw a raycast to the player to see if we can see them
// Returns success if we see them, or failure if not
ResultType behSeePlayer(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    std::vector<int> playerIDs = gctx.gameManager.getPlayerIDs();
    if(playerIDs.size()>0){
        float distance = std::numeric_limits<float>::max();
        NodeComponent* aiComponent = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
        Vector3f pos = aiComponent->node->getPosition();
        for(unsigned int i = 0; i <playerIDs.size();++i)
        {
            Vector3f playerPos = getPlayerPos(gctx, playerIDs[i]);    
            if((pos-playerPos).length()<distance){
                cmp.hittingPlayer = playerIDs[i]; 
                distance = (pos-playerPos).length();
            }   
        }
    }

    Vector3f playerPos = getPlayerPos(gctx, cmp.hittingPlayer);
    Vector3f pos = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID())->node->getPosition();

    bool playerOnSight = gctx.graphicsEngine.checkRayCastCollision(
        pos,
        (playerPos - pos).normalize(),
        cmp.viewDistance,
        MASK MASK_PLAYER
    );
 
    if(playerOnSight)
    {
        bool mapInFrontOfPlayer = gctx.graphicsEngine.checkRayCastCollision(
            pos,
            (playerPos - pos).normalize(),
            (playerPos - pos).length(),
            MASK MASK_MAP
        );
        playerOnSight = !mapInFrontOfPlayer;
    }
    if(playerOnSight)
    {
        //aux->weapons.back().isFiring = true;
        cmp.destination = playerPos;
        cmp.playerLastKnownPos = playerPos;
        cmp.isAggroed = true;

        // Change steering behaviour
        auto& steer = cmp.steeringBehaviours;
        steer.clear();
        steer.push_back(SB_SEPARATION);
        steer.push_back(SB_SEEK);

        cmp.currentPath.clear();
        
        return BT_SUCCESS;
    }
    else
    {
        return BT_FAILURE;
    }

    return BT_SUCCESS;
}

/*Behaviour (Leaf): Patrol*/
// We ask for a waypoint path, and if we have it we move between its nodes
ResultType behPatrol(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    // If we don't have a patrol path, get it
    if(!cmp.patrolPath || cmp.patrolPath->size() == 0)
        cmp.patrolPath = gctx.levelManager.getRoute(cmp.pathIndex);

    // If we have a patrol path, go to next waypoint
    if(cmp.currentPath.size()==0 && cmp.patrolPath && cmp.patrolPath->size() > 0)
    {
        // Check reset patrol path
        if(cmp.currentPatrolPath < 0 || cmp.currentPatrolPath >= (int) cmp.patrolPath->size())
            cmp.currentPatrolPath = 0;

        // Get next waypoint
        Waypoint* tmp { cmp.patrolPath->at(cmp.currentPatrolPath) };
        if(tmp) cmp.destination = tmp->position;

        if(++cmp.currentPatrolPath >= (int)cmp.patrolPath->size())
            cmp.currentPatrolPath = 0;
        return BT_SUCCESS;
    }
    return BT_FAILURE;
}

/*Behaviour (Leaf): Is Alert On*/
// Checks if we have a last known position of the player, if we don't see it
ResultType behIsAlertOn(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    if(cmp.isAggroed)
    {
        cmp.destination = cmp.playerLastKnownPos;
        return BT_SUCCESS;
    }
    return BT_FAILURE;
}

/*Behaviour (Leaf): ALERT Off*/
// Turns the alarm off after looking for the player
ResultType behAlertOff(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    if(cmp.isAggroed)
    {
        cmp.isAggroed = false;

        // Change steering behaviour
        auto& steer = cmp.steeringBehaviours;
        steer.clear();
        steer.push_back(SB_SEPARATION);
        steer.push_back(SB_ARRIVE);

        return BT_SUCCESS;
    }
    
    return BT_FAILURE;
}

/*Behaviour (Leaf): Close To*/
// Checks if we are close to the objective (player)
// Returns success if we are "close" (<=desiredDistance) or failure if not
ResultType behCloseTo(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    Vector3f playerPos = getPlayerPos(gctx, cmp.hittingPlayer);
    Vector3f pos = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID())->node->getPosition();

    float distance = (playerPos - pos).length();

    if(distance <= cmp.aggroDistance)
    {
        cmp.currentPath.clear();

        return BT_SUCCESS;
    }
    if(cmp.behType != KAMIKAZE_TREE && std::abs(playerPos.y-pos.y)<0.1f)
        cmp.destination = playerPos + (pos-playerPos).normalize() * cmp.aggroDistance * 0.8f;
    return BT_FAILURE;
}

/*Behaviour (Leaf): Can Attack*/
// Returns success if our attack current cooldown is <=0, or failure if not
ResultType behCanAttack(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    if(cmp.attackCurrentCooldown <= 0.0f)
        return BT_SUCCESS;
    return BT_FAILURE;
}

/*Behaviour (Leaf): Begin Attack (Melee)*/
// Sets the attack animation time to start
ResultType behBeginAttack(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    int entID { (int)cmp.getEntityID() };

    // Init attack data
    cmp.isAttacking = true;
    cmp.attackTimeCounter = 0.0f;
    cmp.currentPath.clear();
    gctx.entityManager.getComponentByID<CollisionComponent>(entID)->acceleration = {0,0,0};

    // Get data to face the player
    NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(entID);
    Vector3f pos = nodeCmp->node->getPosition();
    Vector3f playerPos = getPlayerPos(gctx, cmp.hittingPlayer);

    // Face the player
    Vector3f direction = playerPos - pos;
    direction.y = 0;
    cmp.lookAt = direction.normalize();

    // Rotate
    Vector3f rot = cmp.lookAt.getHorizontalAngle();
    nodeCmp->node->setRotation(rot);

    // Play animation
    std::string anim { "attack" };
    bool looped { true };
    nodeCmp->node->playAnimation(anim, looped);

    // TODO:: ID in server is ID in host
    auto* typeCmp { gctx.entityManager.getComponentByID<TypeComponent>(entID) };
    int idInServ { typeCmp->idInServer };
    gctx.networkClient.send(
        "EntityAnimationChanged" SEP
        +std::to_string(idInServ)
        +SEP+anim
        +SEP+std::to_string(looped)
    );

    return BT_SUCCESS;
}

/*Behaviour (Leaf): Is Attacking (Melee)*/
// If the attack has started , it returns SUCCESS , if it has not it returns FAILURE 
// It works as a condition to damage the player is behAttack
ResultType behIsAttacking(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    if(cmp.isAttacking)
        return BT_SUCCESS;
    return BT_FAILURE;
}

/*Behaviour (Leaf): Attack (Melee)*/
// Creates an "attack raycast", checks if it collides with player and resets cooldown
// This is done only on certain frames (during the attack window)
// Returns success if player was hit, failure if not or running if the attack is ongoing
ResultType behAttack(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    // Check if attack is still running
    if(cmp.attackTimeCounter < cmp.maxAttackTime)
    {
        return BT_RUNNING;
    }
    else
    {
        // We should check if it's success or failure
        return BT_SUCCESS;
    }
}

/*Behaviour (Leaf): Shoot (Shooter)*/
// Prepares the enemy for shooting (face the player etc) and enables the isFiring of the weapon
// Always returns SUCCESS
ResultType behShoot (TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{

    WeaponComponent* weaponCmp = gctx.entityManager.getComponentByID<WeaponComponent>(cmp.getEntityID());

    cmp.currentPath.clear();
    gctx.entityManager.getComponentByID<CollisionComponent>(cmp.getEntityID())->acceleration = {0,0,0};

    // Get data to face the player
    NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    Vector3f pos = nodeCmp->node->getPosition();
    Vector3f playerPos = getPlayerPos(gctx, cmp.hittingPlayer);

    // Face the player
    Vector3f direction = playerPos - pos;
    direction.y = 0;
    cmp.lookAt = direction.normalize();

    // Rotate
    Vector3f rot = cmp.lookAt.getHorizontalAngle();
    nodeCmp->node->setRotation(rot);

    weaponCmp->weapons[0].isFiring = true;

    return BT_SUCCESS;
}

/*Behaviour (Leaf): InRange (Shooter)*/
// Checks if the player is in shooting range
// Returns SUCCESS if its in range and FAILURE if its not
ResultType behInRange (TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    NodeComponent* player = gctx.entityManager.getComponentByID<NodeComponent>(cmp.hittingPlayer);
    NodeComponent* ai     = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    WeaponComponent* weaponCmp = gctx.entityManager.getComponentByID<WeaponComponent>(cmp.getEntityID());

    /*float distance =sqrt(((player->node->getPosition().x - ai->node->getPosition().x) * (player->node->getPosition().x - ai->node->getPosition().x)) +
                         ((player->node->getPosition().z - ai->node->getPosition().z) * (player->node->getPosition().z - ai->node->getPosition().z)));*/
    float distance = (ai->node->getPosition()-player->node->getPosition()).length();
    weaponCmp->weapons[0].isFiring = false;

    if(distance<cmp.shootingRange)
        return BT_SUCCESS;
    
    return BT_FAILURE;
   
}

/*Behaviour (Leaf): Stalk (Kamikaze)*/
// Returns the position of the player and assigns it as cmp.destination
// Always returns SUCCESS
ResultType behStalker(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{  
    std::vector<int> playerIDs = gctx.gameManager.getPlayerIDs();
    if(playerIDs.size()>0){
        float distance = std::numeric_limits<float>::max();
        NodeComponent* aiComponent = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
        Vector3f pos = aiComponent->node->getPosition();
        for(int i = 0; i <(int)playerIDs.size();++i)
        {
            Vector3f playerPos = getPlayerPos(gctx, playerIDs[i]);    
            if((pos-playerPos).length()<distance){
                cmp.hittingPlayer = playerIDs[i]; 
                distance = (pos-playerPos).length();
            }   
        }
    }
    cmp.destination = gctx.entityManager.getComponentByID<NodeComponent>(cmp.hittingPlayer)->node->getPosition();
    cmp.playerLastKnownPos = cmp.destination;
    cmp.currentPath.clear();

    return BT_SUCCESS;
}

/*Behaviour (Leaf): Explode (Kamikaze)*/
// Activates the attack of the enemy kamikaze (explosion)
// Always returns SUCCESS
ResultType behExplode(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{
    cmp.isAttacking = true;
    return BT_SUCCESS; 
}

/*Behaviour (Leaf): Can Shoot (Raid Shooter)*/
// Determinates if an enemy raid shooter can shoot
// Return failure if there is an obstacle that doesn't let the enemy shoot the player
// Returns success if otherwise
ResultType behCanShoot(TreeNode& treeNode, AIComponent& cmp, GameContext& gctx)
{   
    Vector3f playerPos = getPlayerPos(gctx, cmp.hittingPlayer);
    Vector3f pos = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID())->node->getPosition();

    bool playerNotOnSight = gctx.graphicsEngine.checkRayCastCollision(
        pos,
        (playerPos-pos).normalize(),
        (playerPos-pos).length(),
        MASK MASK_MAP
    );

    if(playerNotOnSight)
        return BT_FAILURE;
    
    return BT_SUCCESS;

}
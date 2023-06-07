#include "aiBehaviours.hpp"
#include "../components/healthCmpFunc.hpp"

/*AI Behaviour: AI Patroller*/
// Patrols a path
void aiPatroller(AIFunctionParams)
{
    // Reduce attack cooldown
    if(cmp.isAttacking)
        cmp.attackTimeCounter += gctx.graphicsEngine.getTimeDiffInSeconds();
    if(!cmp.isAttacking && cmp.attackCurrentCooldown > 0)
        cmp.attackCurrentCooldown -= gctx.graphicsEngine.getTimeDiffInSeconds();
    
    // Take decision with behaviour tree
    //cmp.behaviourTree.decideAction(cmp, gctx);
    if(!cmp.behaviourTree)
        cmp.behaviourTree = new BehaviourTree(cmp.behType);
    cmp.behaviourTree->decideAction(cmp, gctx);

    // Update attack if neccessary
    if(cmp.isAttacking)
    {
        //Little addition for an exclusive KAMIKAZE behaviour
        if(cmp.behType == KAMIKAZE_TREE)
        {
            //Create explosion
            InteractableComponent *interactablecmp = gctx.entityManager.getComponentByID<InteractableComponent>(cmp.getEntityID());
            interactablecmp->interaction(*interactablecmp, cmp.getEntityID(), gctx);
            //kill IA
            HealthComponent *healthcmp = gctx.entityManager.getComponentByID<HealthComponent>(cmp.getEntityID());
            cf::kill(*healthcmp,gctx);
            healthcmp->totalDeathCounter = 0.f;
            // TODO:: Does not work in multiplayer, guests will see the death anim when exploding
            return;
        }
        // Check if we're on attack window
        if(cmp.attackTimeCounter >= cmp.attackWindowStart && 
            cmp.attackTimeCounter <= cmp.attackWindowEnd)
        {
            int playerID = cmp.hittingPlayer;
            NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
            NodeComponent* playerNode = gctx.entityManager.getComponentByID<NodeComponent>(playerID);
            Vector3f pos = nodeCmp->node->getPosition();
            Vector3f playerPos = playerNode->node->getPosition();

            // Face the player
            Vector3f direction = playerPos - pos;
            direction.y = 0;

            // Check if the target is in our attack direction
            Vector3f rotToPlayer = direction.normalize().getHorizontalAngle();
            float angleDiff = (cmp.lookAt.getHorizontalAngle() - rotToPlayer).y;

            bool playerWasHit = (angleDiff >= -60.0f && angleDiff <= 60.0f);
            if(playerWasHit)
            {
                // Player is in front, now check distance
                float distance = (playerPos - pos).length();
                if(distance > cmp.aggroDistance*1.3f)
                    playerWasHit = false;
            }

            if(playerWasHit)
            {
                HealthComponent* health = gctx.entityManager.getComponentByID<HealthComponent>(playerID);
                if(health)
                {
                    bool attacked = cf::doDamage(*health, gctx, 3, 0.8);
                    //Sound attack
                    //Pending to discuss: Where the sound sounds
                    if(attacked) gctx.audioManager.play3DSound(SOUND_SFX_ENEMYHIT, SOUNDS_SFX_COMBAT, pos);
                }
            }
        }

        if(cmp.attackTimeCounter >= cmp.maxAttackTime)
        {
            // Reset counter for next time
            cmp.attackTimeCounter = 0;
            cmp.isAttacking = false;

            // Turn off animation
            int entID { (int)cmp.getEntityID() };
            std::string anim { "walk" };
            bool looped { true };
            NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(entID);
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

            // Reset Cooldown
            cmp.attackCurrentCooldown = cmp.attackMaxCooldown;
        }
    }

    // Follow the current path
    if(cmp.currentPath.size() > 0)
    {
        // Get components
        NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
        CollisionComponent* collisionCmp = gctx.entityManager.getComponentByID<CollisionComponent>(cmp.getEntityID());
        Vector3f pos = nodeCmp->node->getPosition();   

        // Check if we reached the node
        Vector3f nextNode = cmp.currentPath.front();        
        Vector3f endpos = cmp.currentPath.front();

        Vector3f max_err{5.5,5.5,5.5};

        // If the player is near we use a smaller error
        if(cmp.currentPath.size()==1)
            max_err.set(0.5,0.5,0.5);


        if( std::abs(endpos.x-pos.x)<=max_err.x &&
            std::abs(endpos.z-pos.z)<=max_err.z)
        {
            // Delete first element
            cmp.currentPath.erase(cmp.currentPath.begin());

            // If we reached the end of the path, finish
            if(cmp.currentPath.size() == 0)
            {
                //collisionCmp->baseSpeed = {0,0,0};
                collisionCmp->acceleration = {0,0,0};
                return;
            }
            // Recalculate next node's position
            nextNode = cmp.currentPath.front();
        }

        // Get direction to next graph node
        Vector3f direction = nextNode - pos;
        direction.y = 0;
        cmp.lookAt = direction.normalize();

        // Rotate
        Vector3f rot = cmp.lookAt.getHorizontalAngle();
        nodeCmp->node->setRotation(rot);

        // Move (seek)
        collisionCmp->acceleration = helperCalculateSteering(
            cmp.steeringBehaviours
            ,*collisionCmp
            ,cmp
            ,pos
            ,gctx
        );
    }    
}

/*AI Behaviour: Moving Platform*/
// Move from one side to another when told to
void aiMovingPlatform(AIFunctionParams)
{
    NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    Vector3f posActual  = nodeCmp->node->getPosition();
    Vector3f posFinal   = cmp.posF;
    Vector3f posInicial = cmp.posI;

    if( ((posActual.x>=posFinal.x && posFinal.x>=posInicial.x)||(posActual.x<=posFinal.x && posFinal.x<= posInicial.x)) &&
        ((posActual.y>=posFinal.y && posFinal.y>=posInicial.y)||(posActual.y<=posFinal.y && posFinal.y<= posInicial.y)) &&
        ((posActual.z>=posFinal.z && posFinal.z>=posInicial.z)||(posActual.z<=posFinal.z && posFinal.z<= posInicial.z)) )
    {
        cmp.posF=cmp.posI;
        cmp.posI=posFinal;

        CollisionComponent* collisionCmp = gctx.entityManager.getComponentByID<CollisionComponent>(cmp.getEntityID());
        collisionCmp->speed=Vector3f{0,0,0};
    }
}

void aiHealingPad(AIFunctionParams)
{
    helperGenericPad(cmp, gctx, 1);    
}

void aiAmmoPad(AIFunctionParams)
{
    helperGenericPad(cmp, gctx, 2);    
}

/* HELPER SECTION */
void helperGenericPad(AIComponent& cmp, GameContext& gctx, int padType)
{
    //Aggroed calculate if it has a kit on it
    if(cmp.hasKit==0)
    {
        //We use time in node to calculate the time without healing kit
        cmp.timeWithoutKit += gctx.graphicsEngine.getTimeDiff();
        if(cmp.timeWithoutKit>= cmp.maxtimeWithoutKit)
        {//If the time passed you generate a med kit
            cmp.hasKit=1;
            cmp.timeWithoutKit=0;

            // Obtain our id, or machine id if we have a vending machine
            int parentId = cmp.getEntityID();
            bool hasVendingMachine = false;
            auto* padNodeCmp { gctx.entityManager.getComponentByID<NodeComponent>(parentId) };
            auto* interactableCmp = gctx.entityManager.getComponentByID<InteractableComponent>(parentId);
            if(interactableCmp && interactableCmp->idRemoteToBeInteracted > -1)
            {
                parentId = interactableCmp->idRemoteToBeInteracted;
                hasVendingMachine = true;
            }

            //Create a med kit
            Vector3f posKit { padNodeCmp->node->getPosition() + Vector3f{0,3,0} };
            gctx.gameManager.createKitEntity( posKit, Vector3f {1.5,1.5,1.5}, padType, parentId, hasVendingMachine);
        }
    }
}

const Vector3f helperCalculateSteering(
    std::vector<SteeringBehaviour>& steeringBehaviours
    , const CollisionComponent& colcmp
    , const AIComponent& aicmp
    , const Vector3f& currentPosition
    , GameContext& gctx)
{
    Vector3f result {0,0,0};
    for(auto& steerBeh : steeringBehaviours)
    {
        switch(steerBeh)
        {
            case SB_SEEK:
                result += helperSeek(colcmp, aicmp);
                break;
            case SB_ARRIVE:
                result += helperArrive(colcmp, aicmp, currentPosition);
                break;
            case SB_FLEE:
                result += helperFlee(colcmp, aicmp, currentPosition);
                break;
            case SB_SEPARATION:
                std::vector<Vector3f> otherAIs { helperGetOtherAIPosition(aicmp.getEntityID(), gctx) };
                result += helperSeparation(colcmp, aicmp, currentPosition, otherAIs);
                break;
        }
    }
    return result;
}

const Vector3f helperSeek(const CollisionComponent& colcmp, const AIComponent& aicmp) noexcept
{
    Vector3f acceleration { colcmp.baseAcceleration };
    Vector3f movement { aicmp.lookAt * acceleration.x };
    return movement;
}

const Vector3f helperArrive(const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition) noexcept
{
    auto nextNode = aicmp.currentPath.front();
    float dist { (nextNode - currentPosition).length() };
    float velocity { dist / aicmp.timeToArrive };
    float acceleration { (velocity - colcmp.baseSpeed.length()) / aicmp.timeToArrive };
    Vector3f movement { aicmp.lookAt * acceleration };
    return movement;
}

const Vector3f helperFlee(const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition) noexcept
{
    auto vec = helperArrive(colcmp, aicmp, currentPosition);
    vec *= -1;
    return vec;
}

const Vector3f helperSeparation(const CollisionComponent& colcmp, const AIComponent& aicmp, const Vector3f& currentPosition, std::vector<Vector3f>& targets) noexcept
{
    Vector3f result;

    auto& threshold { aicmp.threshold };
    for(auto& target : targets)
    {
        Vector3f direction { target - currentPosition };
        float distance { direction.length() };
        if(distance < threshold)
        {
            float strength { colcmp.baseAcceleration.x * (threshold - distance) / threshold };
            direction.normalize();
            result += direction * -1 * strength;
        }
    }

    return result;
}

const std::vector<Vector3f> helperGetOtherAIPosition(const EntityID& ownId, GameContext& gctx)
{
    std::vector<Vector3f> otherAIs;
    auto& aiCmps = gctx.entityManager.getComponentVector<AIComponent>();
    for(auto& aicmpiter : aiCmps)
    {
        // Skip ourselves
        if(aicmpiter.getEntityID() == ownId) continue;
        // If it is another enemy, save its position
        auto* tmpTypeCmp = gctx.entityManager.getComponentByID<TypeComponent>(aicmpiter.getEntityID());
        if(tmpTypeCmp && tmpTypeCmp->type == ENT_ENEMY)
        {
            auto* tmpNodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(aicmpiter.getEntityID());
            if(tmpNodeCmp)
                otherAIs.push_back(tmpNodeCmp->node->getPosition());
        }
    }
    return otherAIs;
}

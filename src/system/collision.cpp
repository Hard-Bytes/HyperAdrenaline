#include "collision.hpp"
#include <algorithm>
#include "../components/healthCmpFunc.hpp"

// Constructor
CollisionSystem::CollisionSystem(GameContext& gc)
    : gctx(gc)
{}

// Destructor
CollisionSystem::~CollisionSystem()
{

}

/*Update One*/
void CollisionSystem::updateOne(CollisionComponent& cmp) const noexcept
{
    // Recover and check entity data and Node component (we need it!)
    NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    if(!nodeCmp) return;

    if(cmp.moves && !cmp.collidesWithMap)
    {   
        if(cmp.useSteering)
        {
            cmp.speed.x = cmp.baseSpeed.x;
            cmp.speed.z = cmp.baseSpeed.z;  
        }

        // Get the status of the entity
        StatusComponent* statusComponent = gctx.entityManager.getComponentByID<StatusComponent>(cmp.getEntityID());
        if(statusComponent)
            for(unsigned int i=0; i<statusComponent->status.size(); i++)
                if(statusComponent->status[i].stat==STAT_SLOWED)
                {
                    cmp.speed = cmp.speed * 0.4;
                    break;
                }

        //Get delta time   
        float deltaTimeSec = gctx.graphicsEngine.getTimeDiffInSeconds();

        //How many gravity is applied this frame
        cmp.fallingSpeed.y += cmp.gravity.y * deltaTimeSec;
        Vector3f calculatedFalling = cmp.fallingSpeed * deltaTimeSec;

        Vector3f oldPos = nodeCmp->node->getPosition();
        Vector3f newPos = oldPos;

        // Recalculate speed if dashing
        if(cmp.dashCounter > 0)
        {
            cmp.speed = cmp.storedSpeed * cmp.dashMultiplier;
            cmp.dashCounter -= deltaTimeSec;

            if(cmp.dashCounter)
                cmp.dashCooldown = cmp.dashCooldownTime;
        }
        // Reduce dash cooldown if inactive
        else if(cmp.dashCooldown > 0)
        {
            cmp.dashCooldown -= deltaTimeSec;
        }

        // Assign new speed
        newPos += cmp.speed * deltaTimeSec + calculatedFalling;
        nodeCmp->node->setPosition(newPos);
        nodeCmp->node->overrideCollisionPosition_safe(newPos);

        // Repositionate the camera's target (if target is a camera)
        InputComponent* inputComponent = gctx.entityManager.getComponentByID<InputComponent>(cmp.getEntityID());
        if(inputComponent)
        {
            Vector3f target = nodeCmp->node->getTarget();
            target += newPos - oldPos;

            nodeCmp->node->setTarget(target);
        }

        // Calculate speed with acceleration
        if(cmp.useSteering)
        {
            cmp.baseSpeed += cmp.acceleration * deltaTimeSec;
            cmp.baseSpeed.x = std::clamp(cmp.baseSpeed.x, -10.f, 10.f);
            cmp.baseSpeed.z = std::clamp(cmp.baseSpeed.z, -10.f, 10.f);

            // Cause drag (base speed reduces automatically)
            auto dragX = std::fabs(cmp.baseSpeed.x) * cmp.drag * deltaTimeSec;
            if(dragX > fabs(cmp.baseSpeed.x)) dragX = fabs(cmp.baseSpeed.x);
            if(cmp.baseSpeed.x > 0) cmp.baseSpeed.x -= dragX;
            else                    cmp.baseSpeed.x += dragX;
            auto dragZ = std::fabs(cmp.baseSpeed.z) * cmp.drag * deltaTimeSec;
            if(dragZ > fabs(cmp.baseSpeed.z)) dragZ = fabs(cmp.baseSpeed.z);
            if(cmp.baseSpeed.z > 0) cmp.baseSpeed.z -= dragZ;
            else                    cmp.baseSpeed.z += dragZ;
        }

        // Share movement in network
        this->sendOnline(cmp, *nodeCmp, true);
    }
    else if(cmp.collidesWithMap)
    {
        //Get delta time   
        float deltaTimeSec = gctx.graphicsEngine.getTimeDiffInSeconds();
        
        if(cmp.useSteering)
        {
            cmp.speed.x = cmp.baseSpeed.x;
            cmp.speed.z = cmp.baseSpeed.z;  
        }

        // Get the status of the entity
        StatusComponent* statusComponent = gctx.entityManager.getComponentByID<StatusComponent>(cmp.getEntityID());
        if(statusComponent)
            for(unsigned int i=0; i<statusComponent->status.size(); i++)
                if(statusComponent->status[i].stat==STAT_SLOWED)
                {
                    cmp.speed = cmp.speed * 0.4;
                    break;
                }

        // Recalculate speed if dashing
        if(cmp.dashCounter > 0)
        {
            cmp.speed = cmp.storedSpeed * cmp.dashMultiplier;
            cmp.dashCounter -= deltaTimeSec;

            if(cmp.dashCounter)
                cmp.dashCooldown = cmp.dashCooldownTime;
        }
        // Reduce dash cooldown if inactive
        else if(cmp.dashCooldown > 0)
        {
            cmp.dashCooldown -= deltaTimeSec;
        }

        // Moves and collides with map
        auto controller = nodeCmp->node->getCharacterController();
        controller->setWalkDirection(btVector3{cmp.speed.x, cmp.speed.y, cmp.speed.z} * 0.02f);

        // Move second hitbox
        Vector3f newPos = nodeCmp->node->getPosition();
        nodeCmp->node->overrideCollisionPosition(newPos);

        // Repositionate the camera's target (if target is a camera)
        InputComponent* inputComponent = gctx.entityManager.getComponentByID<InputComponent>(cmp.getEntityID());
        if(inputComponent)
        {
            Vector3f target = nodeCmp->node->getTarget();
            auto diff { newPos - cmp.previousPosition };
            target += diff;

            nodeCmp->node->setTarget(target);
        }
        bool posHasChanged { (cmp.previousPosition - newPos).length() > 0.01f };
        cmp.previousPosition = newPos;

        // Calculate speed with acceleration
        if(cmp.useSteering)
        {
            cmp.baseSpeed += cmp.acceleration * deltaTimeSec;
            cmp.baseSpeed.x = std::clamp(cmp.baseSpeed.x, -10.f, 10.f);
            cmp.baseSpeed.z = std::clamp(cmp.baseSpeed.z, -10.f, 10.f);

            // Cause drag (base speed reduces automatically)
            auto dragX = std::fabs(cmp.baseSpeed.x) * cmp.drag * deltaTimeSec;
            if(dragX > fabs(cmp.baseSpeed.x)) dragX = fabs(cmp.baseSpeed.x);
            if(cmp.baseSpeed.x > 0) cmp.baseSpeed.x -= dragX;
            else                    cmp.baseSpeed.x += dragX;
            auto dragZ = std::fabs(cmp.baseSpeed.z) * cmp.drag * deltaTimeSec;
            if(dragZ > fabs(cmp.baseSpeed.z)) dragZ = fabs(cmp.baseSpeed.z);
            if(cmp.baseSpeed.z > 0) cmp.baseSpeed.z -= dragZ;
            else                    cmp.baseSpeed.z += dragZ;
        }

        if(newPos.y < gctx.levelManager.getMinimumYCoord())
        {
            auto* healthCmp { gctx.entityManager.getComponentByID<HealthComponent>(cmp.getEntityID()) };
            cf::kill(*healthCmp, gctx);
        }

        // Share movement in network
        this->sendOnline(cmp, *nodeCmp, posHasChanged);
    }
    if(cmp.collidesWithEvents)
    {
        // Search event collision entity ID
        int eventEntityID = gctx.graphicsEngine.checkEventTriggerCollision(nodeCmp->node);

        // If ID is valid, an event was triggered
        if(eventEntityID>=0)
            if(InteractableComponent* interactionCmp = gctx.entityManager.getComponentByID<InteractableComponent>(eventEntityID))
                interactionCmp->interaction(*interactionCmp, cmp.getEntityID(), gctx);
    }
    if(cmp.collidesWithAreaEffects)
    {
        // Search event collision entity ID
        int eventEntityID = gctx.graphicsEngine.checkAreaEffectCollision(nodeCmp->node);

        // If ID is valid, an area effect was triggered
        if(eventEntityID>=0)
            if(InteractableComponent* interactionCmp = gctx.entityManager.getComponentByID<InteractableComponent>(eventEntityID))
                interactionCmp->interaction(*interactionCmp, cmp.getEntityID(), gctx);
    }
}

/*Update All*/
void CollisionSystem::updateAll() const noexcept
{
    auto& components = gctx.entityManager.getComponentVector<CollisionComponent>();
    for(auto& cmp : components)
    {
        this->updateOne(cmp);
    }
}

/*Send Online*/
// Send changes in position and orientation to other players
void CollisionSystem::sendOnline(CollisionComponent& cmp, NodeComponent& nodeCmp, bool posHasChanged) const noexcept
{
    // Only if it is our player OR we are the host and this is not a player
    int playerId = gctx.gameManager.getLocalPlayerID();
    bool isLocalPlayer = playerId == (int)cmp.getEntityID();
    TypeComponent* type = gctx.entityManager.getComponentByID<TypeComponent>(cmp.getEntityID());
    bool hasOnlineId = type && type->idInServer>=0;
    if(
        hasOnlineId && 
        (
            isLocalPlayer ||
            (gctx.networkClient.isHost() && type->type != ENT_PLAYER)
        )
    )
    {
        Vector3f pos = nodeCmp.node->getPosition();
        Vector3f rot = nodeCmp.node->getRotation();
        Vector3f lookAt;

        // If it has AI, get the AI target
        AIComponent* aiCmp = gctx.entityManager.getComponentByID<AIComponent>(cmp.getEntityID());
        if(aiCmp) lookAt = aiCmp->lookAt;
        else      lookAt = nodeCmp.node->getTarget();

        bool rotHasChanged { (cmp.previouslySentRotation - rot).length() > 0.01f };
        bool lookAtHasChanged { (cmp.previouslySentLookAt - lookAt).length() > 0.01f };

        if(posHasChanged || rotHasChanged || lookAtHasChanged)
        {
            cmp.previouslySentRotation = rot;
            cmp.previouslySentLookAt = lookAt;
            gctx.networkClient.send(
                "EntityMoved" SEP
                +std::to_string(type->idInServer)
                +SEP+(pos-cmp.offset).toNetworkString()
                +SEP+lookAt.toNetworkString()
                +SEP+rot.toNetworkString()
            );
        }
    }
}

#include "behaviourFunctions.hpp"

void behPickup(BehaviourFunctionParams)
{
    NodeComponent* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    Vector3f posActual  = nodeCmp->node->getPosition();
    Vector3f posFinal   = cmp.finalPos;
    Vector3f posInicial = cmp.initialPos;

    if( ((posActual.y>=posFinal.y && posFinal.y>=posInicial.y)||(posActual.y<=posFinal.y && posFinal.y<= posInicial.y)) )
    {
        cmp.finalPos   = cmp.initialPos;
        cmp.initialPos = posFinal;

    }else{
       if(posInicial.y > posFinal.y)
            nodeCmp->node->setPosition(posActual- Vector3f {0,0.5,0}*gctx.graphicsEngine.getTimeDiffInSeconds());
       else
            nodeCmp->node->setPosition(posActual+ Vector3f {0,0.5,0}*gctx.graphicsEngine.getTimeDiffInSeconds());
    }
    cmp.lookAt.y+=25*gctx.graphicsEngine.getTimeDiffInSeconds();
    nodeCmp->node->setRotation( cmp.lookAt);
}

void behKillingMyselfByTime(BehaviourFunctionParams)
{
    cmp.initialPos.y+=gctx.graphicsEngine.getTimeDiffInSeconds();
    if(cmp.initialPos.x<=cmp.initialPos.y)
    {
        gctx.entityManager.markAsDead(cmp.getEntityID());
        if(cmp.sound.size()>0)
        {
            gctx.audioManager.setSoundParameter(cmp.sound,"isFliying",0, SOUNDS_SFX_COMBAT); 
        }
    }
}

void behKillingMyselfByTimeAndCollision(BehaviourFunctionParams)
{
    NodeComponent* entity=gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    if(gctx.graphicsEngine.checkIsCollidingWithMap(entity->node))
    {
        gctx.entityManager.markAsDead(cmp.getEntityID());
        if(cmp.sound.size()>0)
        {
            gctx.audioManager.setSoundParameter(cmp.sound,"isFliying",0, SOUNDS_SFX_COMBAT); 
        }
    }

    behKillingMyselfByTime( cmp, gctx);
}

void behTriggerMyselfByCollision(BehaviourFunctionParams)
{
    NodeComponent* entity=gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    InteractableComponent* interaction=gctx.entityManager.getComponentByID<InteractableComponent>(cmp.getEntityID());
    if(gctx.graphicsEngine.checkIsCollidingWithMap(entity->node))
        interaction->interaction(*interaction, UNREACHABLE_ENTITY, gctx);   //better wording pending
}

void behScalateMyselfAndDieByTime(BehaviourFunctionParams)
{
    float dt= gctx.graphicsEngine.getTimeDiffInSeconds();
    NodeComponent* nodeCmp=gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    Vector3f scale=nodeCmp->node->getScale()*(1.+3.5*dt);
    nodeCmp->node->setScale(scale);
    
    
    InteractableComponent* interactableCmp=gctx.entityManager.getComponentByID<InteractableComponent>(cmp.getEntityID());
    interactableCmp->interactionData-=interactableCmp->interactionData2*dt;
    if(interactableCmp->interactionData < interactableCmp->interactionData2/4)
        interactableCmp->interactionData-=interactableCmp->interactionData2/4;

    behKillingMyselfByTime(cmp,gctx);
}

void behMenuCameraRotation(BehaviourFunctionParams)
{
    auto* nodeCmp = gctx.entityManager.getComponentByID<NodeComponent>(cmp.getEntityID());
    auto target = nodeCmp->node->getTarget() - nodeCmp->node->getPosition();
    auto relativerot = target.getHorizontalAngle();
    auto amount = 5 * gctx.graphicsEngine.getTimeDiffInSeconds();
    relativerot.y += amount;
    target.set(.0f,.0f,.5f);

    // Modifies target
    Vector3f tmp { target };
    gctx.graphicsEngine.calculateCameraMovementDirection(
        false, relativerot, tmp, target
    );

    nodeCmp->node->setTarget(nodeCmp->node->getPosition() + target);
}

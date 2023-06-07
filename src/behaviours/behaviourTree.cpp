#include "behaviourTree.hpp"
#include "../components/aiCmp.hpp"
#include "../context/gameContext.hpp"

BehaviourTree::BehaviourTree(TREE_TYPE type)
{
    buildTree(type);
}

BehaviourTree:: ~BehaviourTree()
{
    tree.clear();
}

void BehaviourTree::decideAction(AIComponent& cmp, GameContext& gctx)
{
    if(tree.size() > 0)
    {
        // Update cap counter
        this->updateCounter += gctx.graphicsEngine.getTimeDiffInSeconds();
        if(updateCounter >= maxCounter)
        {
            updateCounter -= maxCounter;
            // Ping tree
            tree[0].ping(cmp, gctx);
        }
    }
}

void BehaviourTree:: buildTree(TREE_TYPE type)
{
    switch(type)
    {
        case MELEE_TREE :
            buildMeleeTree();
        break;

        case SHOOTING_TREE :
            buildShootingTree();
        break;

        case KAMIKAZE_TREE :
            buildKamikazeTree();
        break;

        case RAID_SHOOTING_TREE :
            buildRaidShootingTree();
        break;

        case RAID_MELEE_TREE :
            buildRaidMeleeTree();
        break;

        default:
            buildMeleeTree();
        break;
    }
    
}

void BehaviourTree::buildMeleeTree()
{  
    tree.reserve(30);

    // Creamos root 1 -> (SELECTOR)
    int root = addNode(nullptr,2,behSelector);
    //Creamos  1.1 (Vemos al jugador (SECUENCIA))
    int seeAndCombat =    addNode(&tree[root],2,behSequence);
    buildMeleeCombatBranch(&tree[seeAndCombat]);
    //Creamos  1.2 (No vemos al jugador (SELECTOR))
    int searchAndPatrol = addNode(&tree[root],2,behSelector);
    buildSearchBranch(&tree[searchAndPatrol]);
   
    
} 

void BehaviourTree::buildShootingTree()
{
    tree.reserve(30);
   
    // Creamos root 1 -> (SELECTOR)
    int root = addNode(nullptr,2,behSelector);
 
    //Creamos  1.1 (Vemos al jugador (SECUENCIA))
    int seeAndCombat =    addNode(&tree[root],2,behSequence);
    buildRangeCombatBranch(&tree[seeAndCombat]);

    //Creamos  1.2 (No vemos al jugador (SELECTOR))
    int searchAndPatrol = addNode(&tree[root],2,behSelector);
    buildSearchBranch(&tree[searchAndPatrol]);

   
}

void BehaviourTree::buildKamikazeTree()
{

    tree.reserve(7);

    // Creamos root 1 -> (SELECTOR)
    int root =        addNode(nullptr,3,behSequence);
    
    /*int stalk =*/   addNode(&tree[root],0,behStalker);

    int succede =     addNode(&tree[root],1,behSucceder);
    
    int sequenece =   addNode(&tree[succede],2,behSequence);

    /*int closeTo = */addNode(&tree[sequenece],0,behCloseTo);

    /*int explode = */addNode(&tree[sequenece],0,behExplode);

    /*int walk = */   addNode(&tree[root],0,behWalk);


}

void BehaviourTree::buildRaidMeleeTree()
{
    tree.reserve(30);

    //Creamos  1.1 (Vemos al jugador (SECUENCIA))
    int seeAndCombat =    addNode(nullptr,1,behSequence);
    buildRaidMeleeCombatBranch(&tree[seeAndCombat]);
}

void BehaviourTree::buildRaidShootingTree()
{

    tree.reserve(30);

    //Creamos  1.1 (Vemos al jugador (SECUENCIA))
    int seeAndCombat =    addNode(nullptr,1,behSequence);
    buildRaidRangeCombatBranch(&tree[seeAndCombat]);


}

void BehaviourTree::buildMeleeCombatBranch(TreeNode* node)
{
    //Creamos  1.1.1 (See player (do we see the player?) (Condition))
    addNode(node,0,behSeePlayer);
    //Creamos  1.1.2 (Attack sequence)
    int attackSucceder = addNode(node,1,behSucceder);
    int meleeCombat =  addNode(&tree[attackSucceder],2,behSelector);
    int attack =  addNode(&tree[meleeCombat],2,behSequence);
    addNode(&tree[attack],0,behIsAttacking);
    attackSucceder = addNode(&tree[attack],1,behSucceder);
    addNode(&tree[attackSucceder],0,behAttack);
    int beginAttack = addNode(&tree[meleeCombat],3,behSequence);
    //Creamos  1.1.2.1 (Estoy a distancia de pegar? (Selector))
    int closeTo = addNode(&tree[beginAttack],2,behSelector);
    //Creamos  1.1.2.2 (Puedo pegar? (Cooldown))
    addNode(&tree[beginAttack],0,behCanAttack);
    //Creamos  1.1.2.3 (Atacar (proyectar hitbox un frame y ver si le damos al playero y resetear cooldown))
    addNode(&tree[beginAttack],0,behBeginAttack);
    //Creamos  1.1.2.1.1 (Estoy a distancia para poder darle? (<= aggroDistance == success))
    addNode(&tree[closeTo],0,behCloseTo);
    //Creamos  1.1.2.1.2 (Acercarse (Walk))
    addNode(&tree[closeTo],0,behWalk);

}

void BehaviourTree::buildRaidMeleeCombatBranch(TreeNode* node)
{
    //Creamos  1.1.1 (See player (do we see the player?) (Condition))
    addNode(node,0,behStalker);
    //Creamos  1.1.2 (Attack sequence)
    int attackSucceder = addNode(node,1,behSucceder);
    int meleeCombat =  addNode(&tree[attackSucceder],2,behSelector);
    int attack =  addNode(&tree[meleeCombat],2,behSequence);
    addNode(&tree[attack],0,behIsAttacking);
    attackSucceder = addNode(&tree[attack],1,behSucceder);
    addNode(&tree[attackSucceder],0,behAttack);
    int beginAttack = addNode(&tree[meleeCombat],3,behSequence);
    //Creamos  1.1.2.1 (Estoy a distancia de pegar? (Selector))
    int closeTo = addNode(&tree[beginAttack],2,behSelector);
    //Creamos  1.1.2.2 (Puedo pegar? (Cooldown))
    addNode(&tree[beginAttack],0,behCanAttack);
    //Creamos  1.1.2.3 (Atacar (proyectar hitbox un frame y ver si le damos al playero y resetear cooldown))
    addNode(&tree[beginAttack],0,behBeginAttack);
    //Creamos  1.1.2.1.1 (Estoy a distancia para poder darle? (<= aggroDistance == success))
    addNode(&tree[closeTo],0,behCloseTo);
    //Creamos  1.1.2.1.2 (Acercarse (Walk))
    addNode(&tree[closeTo],0,behWalk);

}

void BehaviourTree:: buildRaidRangeCombatBranch(TreeNode* node)
{
    //Creamos  1.1.1 (See player (do we see the player?) (Condition))
    addNode(node,0,behStalker);
    //Creamos  1.1.2 (Attack sequence)
    int selector = addNode(node,2,behSelector);
    int sequence = addNode(&tree[selector],3,behSequence);
    addNode(&tree[sequence],0,behInRange);
    addNode(&tree[sequence],0,behCanShoot);
    addNode(&tree[sequence],0,behShoot);

    addNode(&tree[selector],0,behWalk);

}

void BehaviourTree::buildRangeCombatBranch(TreeNode* node)
{
    //Creamos  1.1.1 (See player (do we see the player?) (Condition))
    addNode(node,0,behSeePlayer);
    //Creamos  1.1.2 (Attack sequence)
    int sequence = addNode(node,2,behSequence);
    addNode(&tree[sequence],0,behInRange);
    addNode(&tree[sequence],0,behShoot);
}

void BehaviourTree::buildSearchBranch(TreeNode* node)
{
    buildLastPositionBranch(node);
    buildPatrolBranch(node);
    
}

void BehaviourTree::buildLastPositionBranch(TreeNode* node)
{
    //Creamos  1.2.1 (Buscar (SECUENCIA))
    int search = addNode(node,3,behSequence);
    
    //Creamos  1.2.1.1
    addNode(&tree[search],0,behIsAlertOn);
    //Creamos  1.2.1.2
    int searchWalk = addNode(&tree[search],1,behSucceder);
    //Creamos  1.2.1.3
    int inverseAlertOff = addNode(&tree[search],1,behInverter);
    //Creamos  1.2.1.2.1
    addNode(&tree[searchWalk],0,behWalk);
    //Creamos  1.2.1.3.1
    addNode(&tree[inverseAlertOff],0,behAlertOff);
}

void BehaviourTree::buildPatrolBranch(TreeNode* node)
{
    //Creamos  1.2.2 (Patrulla (Moverse por el sitio) (Comportamiento))
    int patrol = addNode(node,2,behSequence);
    //Creamos  1.2.2.1
    addNode(&tree[patrol],0,behPatrol);    //Hijo 2 -> Patrulla (Moverse por el sitio) (Comportamiento)
    //Creamos  1.2.2.2
    addNode(&tree[patrol],0,behWalk);      //Hijo 3 -> Walk to position (Comportamiento)
    
}

/*Add node*/
// Adds a node with the specified parameters and
// returns its index in the lineal vector
template <class... Args>
int BehaviourTree::addNode(Args&&... args)
{
    tree.emplace_back(std::forward<Args>(args)...);
    return (int)tree.size()-1;
}

#include "aiCmp.hpp"
#include "../behaviours/behaviourTree.hpp"

AIComponent::AIComponent(EntityID entID, TREE_TYPE t,AIBehaviourFunction behfunc, Vector3f p_posI, Vector3f p_posF,int p_pathIndex): 
    Component(entID), 
    aiFunction(behfunc),  
    posI(p_posI), 
    posF(p_posF),
    behType(t),
    //behaviourTree(behType),
    pathIndex(p_pathIndex)
{}

AIComponent::~AIComponent()
{
    if(patrolPath)
        patrolPath = nullptr;
    currentPath.clear();
}

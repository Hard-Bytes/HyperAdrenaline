#include "behaviourCmp.hpp"

BehaviourComponent::BehaviourComponent(EntityID entID, BehaviourFunction behfunc)
    : Component(entID), behaviourFunction(behfunc)
{

}

BehaviourComponent::~BehaviourComponent()
{
}

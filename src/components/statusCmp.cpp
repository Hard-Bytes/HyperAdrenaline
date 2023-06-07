#include "statusCmp.hpp"

StatusComponent::StatusComponent(EntityID entID)
    : Component(entID)
{

}

StatusComponent::~StatusComponent()
{
    status.clear();
}

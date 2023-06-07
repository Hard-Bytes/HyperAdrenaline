#include "healthCmp.hpp"

HealthComponent::HealthComponent(EntityID entID, int p_maxHealth)
    : Component(entID), maxHealth(p_maxHealth), health(p_maxHealth)
{
}

HealthComponent::~HealthComponent()
{
}

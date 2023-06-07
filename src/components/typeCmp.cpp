#include "typeCmp.hpp"

TypeComponent::TypeComponent(EntityID entID, EntityType p_type)
    : Component(entID), type(p_type)
{}

TypeComponent::~TypeComponent()
{}

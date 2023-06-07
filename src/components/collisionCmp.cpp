#include "collisionCmp.hpp"

CollisionComponent::CollisionComponent(
    EntityID entID,
    Vector3f p_size,
    Vector3f p_gravity,
    Vector3f p_baseAccel,
    Vector3f p_offset,
    Vector3f p_fallingSpeed
)
    : Component(entID)
    , size(p_size)
    , speed{0,0,0}
    , acceleration{0,0,0}
    , gravity(p_gravity)
    , baseSpeed{0,0,0}
    , baseAcceleration(p_baseAccel)
    , fallingSpeed(p_fallingSpeed)
    , offset(p_offset)
{}

CollisionComponent::~CollisionComponent()
{

}

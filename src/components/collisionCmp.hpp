#pragma once
#include "component.hpp"

struct CollisionComponent : public Component
{
    // Constructor
    explicit CollisionComponent(
        EntityID entID,
        Vector3f p_size,
        Vector3f p_gravity={0,0,0},
        Vector3f p_baseAccel={0,0,0},
        Vector3f p_offset={0,0,0},
        Vector3f p_fallingSpeed={0,0,0}
    );
    ~CollisionComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeCollision; }

    bool moves                   {false};
    bool collidesWithMap         {false};
    bool collidesWithAreaEffects {false};
    bool collidesWithEvents      {false};

    Vector3f size;
    Vector3f speed;
    Vector3f acceleration;
    Vector3f gravity;
    Vector3f baseSpeed;
    Vector3f baseAcceleration;
    Vector3f maxBaseSpeed;
    Vector3f fallingSpeed;
    Vector3f offset;
    
    Vector3f previousPosition;

    Vector3f previouslySentRotation;
    Vector3f previouslySentLookAt {0,0,1};

    float drag = 7.0f;
    bool useSteering {false};

    float dashCounter{0};
    float dashTime{0.2f};
    float dashMultiplier{7};
    float dashCooldown{0};
    float dashCooldownTime{1.3f};
    Vector3f storedSpeed;
};

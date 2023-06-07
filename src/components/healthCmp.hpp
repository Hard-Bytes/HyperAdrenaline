#pragma once
#include "component.hpp"

struct HealthComponent : public Component
{
    // Constructor
    explicit HealthComponent(EntityID entID, int p_maxHealth);
    ~HealthComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeHealth; }

    float maxHealth             {10.0f};
    float health                {10.0f};
    float inmunityCooldown      {0.0f};
    float maxInmunityCooldown   {2.0f};

    float totalDeathCounter { 1.f };
    bool totalDeathStarted { false };
};

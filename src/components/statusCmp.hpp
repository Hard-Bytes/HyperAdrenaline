#pragma once
#include "component.hpp"

enum StatusType
{
    STAT_NORMAL = 0,
    STAT_SLOWED,
    STAT_BURNING,
    STAT_KNOCKED
};
struct Stat
{
    StatusType stat;
    //This identify who make the stat, bc if the entity dies for the stat someone need to take the points
    int statCause {-1};
    float maxTimeAtStat {2.5f};
    float timeAtStat {0.f};
    float statCounter {0.f};
};

struct StatusComponent : public Component
{
    // Constructor
    explicit StatusComponent(EntityID entID);
    ~StatusComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeStatus; }

    std::vector<Stat> status{}; 
};

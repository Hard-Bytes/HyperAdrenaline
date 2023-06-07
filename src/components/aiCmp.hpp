#pragma once
#include <list>
#include "component.hpp"
#include "behaviourFunctions.hpp"
#include "../behaviours/behaviourTree.hpp"
#include "../pathfinding/waypoint.hpp"
#include "../util/enums.hpp"

enum SteeringBehaviour
{
     SB_SEEK
    ,SB_ARRIVE
    ,SB_FLEE
    ,SB_SEPARATION
};

struct AIComponent : public Component
{
    // Constructor
    explicit AIComponent(EntityID entID, TREE_TYPE t, AIBehaviourFunction behfunc=nullptr, Vector3f p_posI={0,0,0},Vector3f p_posF={0,0,0}, int p_pathIndex=0);
    ~AIComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeAI; }

    AIBehaviourFunction aiFunction;

    Vector3f posI,posF;

    // Behaviour
    TREE_TYPE behType;
    BehaviourTree* behaviourTree {nullptr};

    // World information
    Vector3f lookAt{0,0,-1};
    Vector3f destination{0,0,0};

    // Steering behaviour data
    float timeToArrive {0.2f};
    float threshold { 2.0f };
    std::vector<SteeringBehaviour> steeringBehaviours { SB_ARRIVE };

    //Created in raid
    bool isRaidEnemy{false};
    
    // Combat information
    int hittingPlayer{0};
    float viewDistance { 100.0f };
    float aggroDistance { 4.0f };
    Vector3f playerLastKnownPos{0,0,0};
    bool isAggroed { false };
    bool isAttacking { false };
    float attackCurrentCooldown{0}; // (seconds) Current time that we must wait to attack again
    float attackMaxCooldown{2};     // (seconds) Cooldown of the attack (max value used to reset after attacking)

    float attackTimeCounter{0};
    float maxAttackTime{1.0f};
    float attackWindowStart{0.3f};
    float attackWindowEnd{1.f};

    float shootingRange {25.0f};

    // Patrol information
    std::vector<Waypoint*>* patrolPath { nullptr };
    int currentPatrolPath{0};
    int pathIndex{0};

    // Pathfinding information
    std::list<Vector3f> currentPath{};

    // HealPad information
    int hasKit { 0 };
    float timeWithoutKit {0.0f};
    float maxtimeWithoutKit{500}; // 10000ms == 10s
};


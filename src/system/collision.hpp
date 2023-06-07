#pragma once
#include "../context/gameContext.hpp"

class CollisionSystem
{
public:
    // Constructor
    explicit CollisionSystem(GameContext& gc);
    ~CollisionSystem();

    void updateOne(CollisionComponent& cmp) const noexcept;
    void updateAll() const noexcept;
private:
    GameContext& gctx;

    void sendOnline(CollisionComponent& cmp, NodeComponent& nodeCmp, bool hasChangedPos) const noexcept;
};

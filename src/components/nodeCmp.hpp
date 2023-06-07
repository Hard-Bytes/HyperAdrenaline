#pragma once
#include "../facade/graphicsEngine.hpp"
#include "../facade/vector3f.hpp"
#include "component.hpp"

struct NodeComponent : public Component
{
    // Constructor
    explicit NodeComponent(EntityID entID, GraphicNode* p_node);
    ~NodeComponent();

    inline static ComponentID getComponentTypeID(){ return (ComponentID)TypeNode; }

    GraphicNode* node {nullptr};
};

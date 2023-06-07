#include "nodeCmp.hpp"
#include "../util/macros.hpp"

NodeComponent::NodeComponent(EntityID entID, GraphicNode* p_node)
    : Component(entID), node(p_node)
{
    // Save the ID in the name to use later
    node->setName(entID);
}

NodeComponent::~NodeComponent()
{
    // delete node;
}

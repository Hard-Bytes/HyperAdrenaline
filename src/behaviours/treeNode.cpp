#include "treeNode.hpp"

TreeNode:: TreeNode (TreeNode* dad,int childNum, BehaviourTreeNodeAction p_action):
    parent(dad),
    action(p_action)
{
    childs.reserve(childNum);
    if(parent)
        parent->appendChild(this);
}

TreeNode:: ~TreeNode()
{
    childs.clear();
}

constexpr void TreeNode::init() noexcept
{
    
}

ResultType TreeNode::ping(AIComponent& cmp, GameContext& gctx) noexcept
{
    return this->action(
        *this,
        cmp,
        gctx
    );
}

void TreeNode:: appendChild( TreeNode* newChild) noexcept
{
    childs.push_back(newChild);
}

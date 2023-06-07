#pragma once
#include <iostream>
#include <vector>
#include "behaviourNodeActions.hpp"

class TreeNode
{
public:
    // Constructor
    explicit TreeNode(TreeNode* dad,int childNum, BehaviourTreeNodeAction p_action);
    ~TreeNode();

    TreeNode* parent;
    std::vector<TreeNode*> childs;

    constexpr void init() noexcept;
    ResultType ping(AIComponent&, GameContext& gctx) noexcept;
    
    void appendChild(TreeNode* newChild) noexcept;
    
private:
    BehaviourTreeNodeAction action;
};

#pragma once
#include "treeNode.hpp"
#include "../util/enums.hpp"

class BehaviourTree
{

public:

    // Constructor
    explicit BehaviourTree(TREE_TYPE);
    ~BehaviourTree();

    void decideAction(AIComponent&, GameContext& );

private:
    std::vector<TreeNode> tree;

    float maxCounter{0.5f}; // const
    float updateCounter{maxCounter};

    //Tree building
    void buildTree(TREE_TYPE);
    void buildMeleeTree();
    void buildShootingTree();
    void buildKamikazeTree();
    void buildRaidMeleeTree();
    void buildRaidShootingTree();

    //Branch building
    void buildMeleeCombatBranch(TreeNode* node);
    void buildRaidMeleeCombatBranch(TreeNode* node);
    void buildRangeCombatBranch(TreeNode* node);
    void buildRaidRangeCombatBranch(TreeNode* node);
    void buildSearchBranch(TreeNode* node);
        void buildLastPositionBranch(TreeNode* node);
        void buildPatrolBranch(TreeNode* node);
    

    // Add node
    template <class... Args>
    int addNode(Args&&... args);
};

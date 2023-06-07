#pragma once

class TreeNode;
struct AIComponent;
class GameContext;

enum ResultType
{
    BT_SUCCESS = 0,
    BT_FAILURE,
    BT_RUNNING
};

typedef ResultType (*BehaviourTreeNodeAction)
(
    TreeNode& treeNode,
    AIComponent& cmp,
    GameContext& gctx
);
